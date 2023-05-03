#pragma once
#include <basic_includes.h>
#include "boot/limine.h"
#include "boot/terminal/terminal.h"
#include "mm/paging/paging.h"

typedef struct {
    uint8_t* base;
    uint64_t free;
    uint64_t size;
} __attribute__((packed)) bitmap_stat_t;

uint64_t get_free_mem();
void init_pmm();
void* pmalloc(size_t pages);
void pfree(void* ptr, size_t pages);