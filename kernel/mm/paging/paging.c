#include "paging.h"
#include "mm/pmm/pmm.h"
#include "boot/limine.h"
#include "drivers/vga/vga_print.h"

void* page_direct_base = (void*)0xFFFF800000000000;
void* kheap_base       = (void*)0xFFFF900000000000;
void* vmap_base        = (void*)0xFFFFA00000000000;
void* kstack_base      = (void*)0xFFFFB00000000000;
void* kernel_base      = (void*)0xFFFFFFFF80000000;

static uint8_t check_flag(uint64_t* entry_vptr, uint64_t flag) {
    return !(flag ^ (*entry_vptr & flag));
}

static void set_flag(uint64_t* entry_vptr, uint64_t flag) {
    *entry_vptr = *entry_vptr | flag;
}

static void clear_flag(uint64_t* entry_vptr, uint64_t flag) {
    *entry_vptr = *entry_vptr & (~flag);
}

void init_paging() {
    update_bitmap_base((uint64_t)page_direct_base);
    remove_entry(get_cr3(), 4, 0);
    return;
}

void* alloc_table() {
    return pmalloc(1);
}

void free_table(void* table_pptr) {
    pfree(table_pptr, 1);
}

void add_entry(void* table_vptr, uint16_t entry_num, void* entry_pptr, uint64_t flags) {
    uint64_t* entry_vptr = (uint64_t*)(((uint64_t*)table_vptr)[entry_num] + (uint64_t)page_direct_base);
    *entry_vptr = ((uint64_t)entry_pptr & (~0xFFF)) | flags;
}

void update_entry(void* table_vptr, uint16_t entry_num, void* entry_pptr, uint64_t flags) {
    uint64_t* entry_vptr = (uint64_t*)(((uint64_t*)table_vptr)[entry_num] + (uint64_t)page_direct_base);
    *entry_vptr = ((uint64_t)entry_pptr & (~0xFFF)) | flags;
}

void remove_entry(void* table_vptr, uint16_t level, uint16_t entry_num) {
    uint64_t* entry = (uint64_t*)((uint64_t)table_vptr + 8 * entry_num);

    if (level == 1) {
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

void* vmalloc(void* vptr, size_t pages, uint64_t flags);

void vfree(void* vptr, size_t pages);