#pragma once
#include <basic_includes.h>

int   mm_init();
void* mm_malloc(size_t size);
void  mm_free(void* ptr);
void* mm_realloc(void* ptr, size_t size);