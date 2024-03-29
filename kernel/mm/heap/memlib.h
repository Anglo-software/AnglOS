#pragma once
#include <basic_includes.h>

#define MAX_HEAP  (1 << 25)
#define ALIGNMENT 16

int initKernelHeapMem();
void mem_deinit();
void* mem_sbrk(int incr);
void mem_reset_brk();
void* mem_heap_lo();
void* mem_heap_hi();
size_t mem_heapsize();
size_t mem_pagesize();
