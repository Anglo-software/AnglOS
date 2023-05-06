#include "paging.h"
#include "mm/pmm/pmm.h"
#include "boot/limine.h"

void* page_direct_base = 0xFFFF800000000000;
void* kheap_base       = 0xFFFF900000000000;
void* vmap_base        = 0xFFFFA00000000000;
void* kstack_base      = 0xFFFFB00000000000;
void* kernel_base      = 0xFFFFFFFF80000000;

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
    return;
}

void* alloc_table();

void free_table(void* table_pptr);

int add_entry(void* table_pptr, uint8_t level, uint16_t entry_num, void* entry_pptr, uint64_t flags);

int update_entry(void* table_pptr, uint16_t entry_num, void* entry_pptr, uint64_t flags);

int remove_entry(void* table_pptr, uint16_t entry_num);

void* get_physical_from_virtual(void* vptr);

int update_cr3(void* pptr);

void* vmalloc(void* vptr, size_t pages, uint64_t flags);

void vfree(void* vptr, size_t pages);