#pragma once
#include <basic_includes.h>

int initKernelHeap();
void* kmalloc(size_t size);
void* kcalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);