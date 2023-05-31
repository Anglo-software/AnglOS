#pragma once
#include <basic_includes.h>

void   mem_init(int use_mmap);
void   mem_deinit();
void*  mem_sbrk(int incr);
void   mem_reset_brk(); 
void*  mem_heap_lo();
void*  mem_heap_hi();
size_t mem_heapsize();
size_t mem_pagesize();
