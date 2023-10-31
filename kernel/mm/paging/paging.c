#include "paging.h"
#include "boot/limine.h"
#include "libc/stdio.h"
#include "mm/pmm/pmm.h"
#include "threads/spinlock.h"

void* page_direct_base = (void*)0xFFFF800000000000;
void* kheap_base       = (void*)0xFFFF900000000000;
void* lapic_base       = (void*)0xFFFFA00000000000;
void* ioapic_base      = (void*)0xFFFFA10000000000;
void* kstack_base      = (void*)0xFFFFB00000000000;
void* kernel_base      = (void*)0xFFFFFFFF80000000;

static void* bsp_cr3;
static spinlock lock = SPINLOCK_INITIALIZER;

extern uint64_t mem_size;

void initPaging() {
    pmmUpdateBitmapBase((uint64_t)page_direct_base);
    vfree((void*)0, mem_size / PAGE_SIZE, false);
    bsp_cr3 = pagingGetCR3();
    return;
}

void initAPPaging() {
    pagingSetCR3((void*)((uint64_t)bsp_cr3 - (uint64_t)page_direct_base));
}

#define NUM_ALLOC 64

static void* bulk_alloc_ptrs[NUM_ALLOC];

static uint8_t curr = 63;

static void* pmallocBulk() {
    if (curr + 1 == NUM_ALLOC) {
        void* ptr = pmalloc(NUM_ALLOC);
        for (int i = 0; i < NUM_ALLOC; i++) {
            bulk_alloc_ptrs[i] = ptr + i * PAGE_SIZE;
        }
        curr = 0;
    }
    return bulk_alloc_ptrs[curr++];
}

void* pagingAllocTable() {
    void* pptr     = pmallocBulk();
    uint64_t* vptr = (uint64_t*)(pptr + (uint64_t)page_direct_base);
    for (int i = 0; i < PAGE_SIZE / 8; i++) {
        *vptr = 0;
    }
    return pptr;
}

void pagingFreeTable(void* table_pptr) { pfree(table_pptr, 1); }

void pagingAddEntry(void* table_vptr, uint16_t entry_num, void* entry_pptr,
                    uint64_t flags) {
    uint64_t* entry_vptr = (uint64_t*)(table_vptr + entry_num * 8);
    *entry_vptr          = ((uint64_t)entry_pptr & (~0xFFF)) | flags;
}

void pagingUpdateEntry(void* table_vptr, uint16_t entry_num, void* entry_pptr,
                       uint64_t flags) {
    uint64_t* entry_vptr = (uint64_t*)(table_vptr + entry_num * 8);
    *entry_vptr          = ((uint64_t)entry_pptr & (~0xFFF)) | flags;
}

extern uint64_t mem_size;

void pagingRemoveEntry(void* table_vptr, uint16_t level, uint16_t entry_num,
                       bool do_pfree) {
    uint64_t* entry = (uint64_t*)((uint64_t)table_vptr + 8 * entry_num);

    if ((*entry & 0x1) != 0x1) {
        return;
    }

    void* entry_pptr = (void*)(*entry & 0x000FFFFFFFFFF000);

    if ((uint64_t)entry_pptr >= mem_size) {
        return;
    }

    if (level == 1) {
        if (do_pfree) {
            pfree(entry_pptr, 1);
        }
        *entry = 0;
    }

    else {
        for (int i = 0; i < 512; i++) {
            pagingRemoveEntry(entry_pptr + (uint64_t)page_direct_base,
                              level - 1, i, do_pfree);
        }
        pagingFreeTable(entry_pptr);
        *entry = 0;
    }
}

void* pagingGetCR3() {
    uint64_t cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    return (void*)((cr3 & (~0xFFF)) + (uint64_t)page_direct_base);
}

void pagingSetCR3(void* pptr) {
    uint64_t cr3 = (uint64_t)pptr;
    __asm__ volatile("mov cr3, %0" : : "r"(cr3) : "memory");
}

void pagingFlushTLB() {
    pagingSetCR3(pagingGetCR3() - (uint64_t)page_direct_base);
}

#define min(a, b) (a < b ? a : b)

static void* vmallocHelper(void* vptr, uint64_t* p4tp, uint64_t flags) {
    void* pptr;

    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e     = p4tp[p4e_num];
    if (p4e == 0) {
        pptr = pagingAllocTable();
        pagingAddEntry((void*)p4tp, p4e_num, pptr, flags);
    }
    p4e = p4tp[p4e_num];
    uint64_t* p3tp =
        (uint64_t*)((p4e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e     = p3tp[p3e_num];
    if (p3e == 0) {
        pptr = pagingAllocTable();
        pagingAddEntry((void*)p3tp, p3e_num, pptr, flags);
    }
    p3e = p3tp[p3e_num];
    uint64_t* p2tp =
        (uint64_t*)((p3e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e     = p2tp[p2e_num];
    if (p2e == 0) {
        pptr = pagingAllocTable();
        pagingAddEntry((void*)p2tp, p2e_num, pptr, flags);
    }
    p2e = p2tp[p2e_num];
    uint64_t* p1tp =
        (uint64_t*)((p2e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p1e_num = PAGE_P1E((uint64_t)vptr);
    pptr             = pmallocBulk();
    pagingAddEntry((void*)p1tp, p1e_num, pptr, flags);
    return vptr;
}

void* vmalloc(void* vptr, size_t pages, uint64_t flags) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)pagingGetCR3();
    for (size_t i = 0; i < pages; i++) {
        void* ptr = vmallocHelper(vptr + i * PAGE_SIZE, p4tp, flags);
        if (ptr == NULL) {
            spinUnlock(&lock);
            return NULL;
        }
    }
    spinUnlock(&lock);
    pagingFlushTLB();
    return vptr;
}

void* vmallocCR3(void* vptr, size_t pages, uint64_t flags, void* cr3) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)cr3;
    for (size_t i = 0; i < pages; i++) {
        void* ptr = vmallocHelper(vptr + i * PAGE_SIZE, p4tp, flags);
        if (ptr == NULL) {
            spinUnlock(&lock);
            return NULL;
        }
    }
    spinUnlock(&lock);
    return vptr;
}

static void vfreeHelper(void* vptr, uint64_t* p4tp, bool do_pfree) {
    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e     = p4tp[p4e_num];
    if (p4e == 0) {
        return;
    }
    uint64_t* p3tp =
        (uint64_t*)((p4e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e     = p3tp[p3e_num];
    if (p3e == 0) {
        return;
    }
    uint64_t* p2tp =
        (uint64_t*)((p3e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e     = p2tp[p2e_num];
    if (p2e == 0) {
        return;
    }
    uint64_t* p1tp =
        (uint64_t*)((p2e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p1e_num = PAGE_P1E((uint64_t)vptr);
    uint64_t p1e     = p1tp[p1e_num];
    pagingRemoveEntry((void*)p1tp, 1, p1e_num, do_pfree);

    for (int i = 0; i < 512; i++) {
        p1e = p1tp[i];
        if (p1e != 0) {
            return;
        }
    }
    pagingRemoveEntry((void*)p2tp, 2, p2e_num, do_pfree);

    for (int i = 0; i < 512; i++) {
        p2e = p2tp[i];
        if (p2e != 0) {
            return;
        }
    }
    pagingRemoveEntry((void*)p3tp, 3, p3e_num, do_pfree);

    for (int i = 0; i < 512; i++) {
        p3e = p3tp[i];
        if (p3e != 0) {
            return;
        }
    }
    pagingRemoveEntry((void*)p4tp, 4, p4e_num, do_pfree);
}

void vfree(void* vptr, size_t pages, bool do_pfree) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)pagingGetCR3();
    for (size_t i = 0; i < pages; i++) {
        vfreeHelper(vptr + i * PAGE_SIZE, p4tp, do_pfree);
    }
    spinUnlock(&lock);
    pagingFlushTLB();
}

void vfreeCR3(void* vptr, size_t pages, bool do_pfree, void* cr3) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)cr3;
    for (size_t i = 0; i < pages; i++) {
        vfreeHelper(vptr + i * PAGE_SIZE, p4tp, do_pfree);
    }
    spinUnlock(&lock);
}

static void* videntityHelper(void* vptr, void* pptr, uint64_t* p4tp,
                             uint64_t flags) {
    void* tmpptr;

    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e     = p4tp[p4e_num];
    if (p4e == 0) {
        tmpptr = pagingAllocTable();
        pagingAddEntry((void*)p4tp, p4e_num, tmpptr, flags);
    }
    p4e = p4tp[p4e_num];
    uint64_t* p3tp =
        (uint64_t*)((p4e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e     = p3tp[p3e_num];
    if (p3e == 0) {
        tmpptr = pagingAllocTable();
        pagingAddEntry((void*)p3tp, p3e_num, tmpptr, flags);
    }
    p3e = p3tp[p3e_num];
    uint64_t* p2tp =
        (uint64_t*)((p3e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e     = p2tp[p2e_num];
    if (p2e == 0) {
        tmpptr = pagingAllocTable();
        pagingAddEntry((void*)p2tp, p2e_num, tmpptr, flags);
    }
    p2e = p2tp[p2e_num];
    uint64_t* p1tp =
        (uint64_t*)((p2e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p1e_num = PAGE_P1E((uint64_t)vptr);
    pagingAddEntry((void*)p1tp, p1e_num, pptr, flags);
    return vptr;
}

void* videntity(void* vptr, void* pptr, size_t pages, uint64_t flags) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)pagingGetCR3();
    for (size_t i = 0; i < pages; i++) {
        void* ptr = videntityHelper(vptr + i * PAGE_SIZE, pptr + i * PAGE_SIZE,
                                    p4tp, flags);
        if (ptr == NULL) {
            return NULL;
            spinUnlock(&lock);
        }
    }
    spinUnlock(&lock);
    pagingFlushTLB();
    return vptr;
}

void* videntityCR3(void* vptr, void* pptr, size_t pages, uint64_t flags, void* cr3) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)cr3;
    for (size_t i = 0; i < pages; i++) {
        void* ptr = videntityHelper(vptr + i * PAGE_SIZE, pptr + i * PAGE_SIZE,
                                    p4tp, flags);
        if (ptr == NULL) {
            return NULL;
            spinUnlock(&lock);
        }
    }
    spinUnlock(&lock);
    return vptr;
}

void vflagsHelper(void* vptr, uint64_t* p4tp, uint64_t flags) {
    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e     = p4tp[p4e_num];
    if (p4e == 0) {
        return;
    }
    p4e = p4tp[p4e_num];
    uint64_t* p3tp =
        (uint64_t*)((p4e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e     = p3tp[p3e_num];
    if (p3e == 0) {
        return;
    }
    p3e = p3tp[p3e_num];
    uint64_t* p2tp =
        (uint64_t*)((p3e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e     = p2tp[p2e_num];
    if (p2e == 0) {
        return;
    }
    p2e = p2tp[p2e_num];
    uint64_t* p1tp =
        (uint64_t*)((p2e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    uint64_t p1e_num = PAGE_P1E((uint64_t)vptr);
    p1tp[p1e_num]    = (p1tp[p1e_num] & 0x8FFFFFFFFFFFFFE0) | flags;
}

void vflags(void* vptr, size_t pages, uint64_t flags) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)pagingGetCR3();
    for (size_t i = 0; i < pages; i++) {
        vflagsHelper(vptr + i * PAGE_SIZE, p4tp, flags);
    }
    spinUnlock(&lock);
    pagingFlushTLB();
}

void vflagsCR3(void* vptr, size_t pages, uint64_t flags, void* cr3) {
    spinLock(&lock);
    uint64_t* p4tp = (uint64_t*)cr3;
    for (size_t i = 0; i < pages; i++) {
        vflagsHelper(vptr + i * PAGE_SIZE, p4tp, flags);
    }
    spinUnlock(&lock);
}

void* pagingCreateUser() {
    uint64_t* p4tp =
        (uint64_t*)((uint64_t)pagingAllocTable() + (uint64_t)page_direct_base);
    uint64_t* ker_cr3 = (uint64_t*)bsp_cr3;
    for (int i = 256; i < 512; i++) {
        p4tp[i] = ker_cr3[i];
    }
    return (void*)p4tp;
}

static void printTable(uint64_t* vptr) {
    int cols = 8;
    for (int i = 0; i < 512 / cols; i++) {
        for (int j = 0; j < cols; j++) {
            kprintf("%lx", vptr[j + cols * i]);
            if (j != cols - 1) {
                kprintf(" ");
            }
        }
        i += cols;
        kprintf("\n");
    }
}

void pagingDumpTable(void* vptr, uint16_t level) {
    uint64_t* p4tp = (uint64_t*)pagingGetCR3();

    if (level == 4) {
        printTable(p4tp);
    }

    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e     = p4tp[p4e_num];
    if (p4e == 0) {
        return;
    }
    uint64_t* p3tp =
        (uint64_t*)((p4e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    if (level == 3) {
        printTable(p3tp);
    }

    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e     = p3tp[p3e_num];
    if (p3e == 0) {
        return;
    }
    uint64_t* p2tp =
        (uint64_t*)((p3e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    if (level == 2) {
        printTable(p2tp);
    }

    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e     = p2tp[p2e_num];
    if (p2e == 0) {
        return;
    }
    uint64_t* p1tp =
        (uint64_t*)((p2e & 0x000FFFFFFFFFF000) + (uint64_t)page_direct_base);

    if (level == 1) {
        printTable(p1tp);
    }
}