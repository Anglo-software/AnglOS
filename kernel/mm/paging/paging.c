#include "paging.h"
#include "mm/pmm/pmm.h"
#include "boot/limine.h"
#include "drivers/vga/vga_print.h"

void* page_direct_base = (void*)0xFFFF800000000000;
void* kheap_base       = (void*)0xFFFF900000000000;
void* vmap_base        = (void*)0xFFFFA00000000000;
void* kstack_base      = (void*)0xFFFFB00000000000;
void* kernel_base      = (void*)0xFFFFFFFF80000000;

void init_paging() {
    update_bitmap_base((uint64_t)page_direct_base);
    //remove_entry(get_cr3(), 4, 0);
    return;
}

#define NUM_ALLOC 64

static void* bulk_alloc_ptrs[NUM_ALLOC];

static uint8_t curr = 63;

static void* pmalloc_bulk() {
    if (curr + 1 == NUM_ALLOC) {
        void* ptr = pmalloc(NUM_ALLOC);
        for (int i = 0; i < NUM_ALLOC; i++) {
            bulk_alloc_ptrs[i] = ptr + i * PAGE_SIZE;
        }
        curr = 0;
    }
    return bulk_alloc_ptrs[curr++];
}

void* alloc_table() {
    void* pptr = pmalloc_bulk();
    uint64_t* vptr = (uint64_t*)(pptr + (uint64_t)page_direct_base);
    for (int i = 0; i < PAGE_SIZE / 8; i++) {
        *vptr = 0;
    }
    return pptr;
}

void free_table(void* table_pptr) {
    pfree(table_pptr, 1);
}

void add_entry(void* table_vptr, uint16_t entry_num, void* entry_pptr, uint64_t flags) {
    uint64_t* entry_vptr = (uint64_t*)(table_vptr + entry_num * 8);
    *entry_vptr = ((uint64_t)entry_pptr & (~0xFFF)) | flags;
}

void update_entry(void* table_vptr, uint16_t entry_num, void* entry_pptr, uint64_t flags) {
    uint64_t* entry_vptr = (uint64_t*)(table_vptr + entry_num * 8);
    *entry_vptr = ((uint64_t)entry_pptr & (~0xFFF)) | flags;
}

void remove_entry(void* table_vptr, uint16_t level, uint16_t entry_num) {
    uint64_t* entry = (uint64_t*)((uint64_t)table_vptr + 8 * entry_num);

    if (level == 1) {
        void* entry_pptr = (void*)(*entry & 0x000FFFFFFFFFF000);
        pfree(entry_pptr, 1);
        *entry = 0;
    }

    else {
        void* entry_pptr = (void*)(*entry & 0x000FFFFFFFFFF000);
        for (int i = 0; i < 512; i++) {
            remove_entry(entry_pptr + (uint64_t)page_direct_base, level - 1, i);
        }
        free_table(entry_pptr);
        *entry = 0;
    }
}

void* get_cr3() {
    uint64_t cr3;
    __asm__ volatile ("mov %0, cr3" : "=r"(cr3));
    return (void*)((cr3 & (~0xFFF)) + (uint64_t)page_direct_base);
}

void set_cr3(void* pptr) {
    uint64_t cr3 = (uint64_t)pptr;
    __asm__ volatile ("mov cr3, %0" : : "r"(cr3));
}

#define min(a, b) (a < b ? a : b)

static void* vmalloc_helper(void* vptr, uint64_t* p4tp, uint64_t flags) {
    void* pptr;

    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e = p4tp[p4e_num];
    if (p4e == 0) {
        pptr = alloc_table();
        add_entry((void*)p4tp, p4e_num, pptr, flags);
    }
    p4e = p4tp[p4e_num];
    uint64_t* p3tp = (uint64_t*)(p4e & 0x000FFFFFFFFFF000);
    
    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e = p3tp[p3e_num];
    if (p3e == 0) {
        pptr = alloc_table();
        add_entry((void*)p3tp, p3e_num, pptr, flags);
    }
    p3e = p3tp[p3e_num];
    uint64_t* p2tp = (uint64_t*)(p3e & 0x000FFFFFFFFFF000);
    
    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e = p2tp[p2e_num];
    if (p2e == 0) {
        pptr = alloc_table();
        add_entry((void*)p2tp, p2e_num, pptr, flags);
    }
    p2e = p2tp[p2e_num];
    uint64_t* p1tp = (uint64_t*)(p2e & 0x000FFFFFFFFFF000);
    
    uint64_t p1e_num = PAGE_P1E((uint64_t)vptr);
    pptr = pmalloc_bulk();
    add_entry((void*)p1tp, p1e_num, pptr, flags);
    return vptr;
}

void* vmalloc(void* vptr, size_t pages, uint64_t flags) {
    uint64_t* p4tp = (uint64_t*)get_cr3();
    for (size_t i = 0; i < pages; i++) {
        void* ptr = vmalloc_helper(vptr + i * PAGE_SIZE, p4tp, flags);
        if (ptr == NULL) {
            return NULL;
        }
    }
    return vptr;
}

static void vfree_helper(void* vptr, uint64_t* p4tp) {
    uint64_t p4e_num = PAGE_P4E((uint64_t)vptr);
    uint64_t p4e = p4tp[p4e_num];
    if (p4e == 0) {
        return;
    }
    p4e = p4tp[p4e_num];
    uint64_t* p3tp = (uint64_t*)(p4e & 0x000FFFFFFFFFF000);
    
    uint64_t p3e_num = PAGE_P3E((uint64_t)vptr);
    uint64_t p3e = p3tp[p3e_num];
    if (p3e == 0) {
        return;
    }
    p3e = p3tp[p3e_num];
    uint64_t* p2tp = (uint64_t*)(p3e & 0x000FFFFFFFFFF000);
    
    uint64_t p2e_num = PAGE_P2E((uint64_t)vptr);
    uint64_t p2e = p2tp[p2e_num];
    if (p2e == 0) {
        return;
    }
    p2e = p2tp[p2e_num];
    uint64_t* p1tp = (uint64_t*)(p2e & 0x000FFFFFFFFFF000);
    
    uint64_t p1e_num = PAGE_P1E((uint64_t)vptr);
    uint64_t p1e = p1tp[p1e_num];
    remove_entry((void*)p1tp, 1, p1e_num);

    for (int i = 0; i < 512; i++) {
        p1e = p1tp[i];
        if (p1e != 0) {
            return;
        }
    }
    remove_entry((void*)p2tp, 2, p2e_num);

    for (int i = 0; i < 512; i++) {
        p2e = p2tp[i];
        if (p2e != 0) {
            return;
        }
    }
    remove_entry((void*)p3tp, 2, p3e_num);

    for (int i = 0; i < 512; i++) {
        p3e = p3tp[i];
        if (p3e != 0) {
            return;
        }
    }
    remove_entry((void*)p4tp, 2, p4e_num);
}

void vfree(void* vptr, size_t pages) {
    uint64_t* p4tp = (uint64_t*)get_cr3();
    for (size_t i = 0; i < pages; i++) {
        vfree_helper(vptr + i * PAGE_SIZE, p4tp);
    }
}