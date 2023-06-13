#pragma once
#include <basic_includes.h>

int   kmeminit();
void* kmalloc(size_t size);
void  kfree(void* ptr);
void* krealloc(void* ptr, size_t size);