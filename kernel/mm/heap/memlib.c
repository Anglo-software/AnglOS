#include "memlib.h"
#include "mm/paging/paging.h"

extern void* kheap_base;

static char* mem_start_brk;
static char* mem_brk;
static char* mem_max_addr;

int initKernelHeapMem() {
    if ((mem_start_brk = (char*)vmalloc(
             kheap_base, MAX_HEAP / PAGE_SIZE,
             PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_EXECDBLE)) ==
        NULL) {
        return -1;
    }
    mem_max_addr = mem_start_brk + MAX_HEAP;
    mem_brk      = mem_start_brk;
    return 0;
}

void mem_deinit() { vfree(mem_start_brk, MAX_HEAP / PAGE_SIZE, true); }

void* mem_sbrk(int incr) {
    char* old_brk = mem_brk;

    if ((incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
        return NULL;
    }
    mem_brk += incr;
    return (void*)old_brk;
}

void mem_reset_brk() { mem_brk = mem_start_brk; }

void* mem_heap_lo() { return (void*)mem_start_brk; }

void* mem_heap_hi() { return (void*)(mem_brk - 1); }

size_t mem_heapsize() { return (size_t)(mem_brk - mem_start_brk); }

size_t mem_pagesize() { return PAGE_SIZE; }