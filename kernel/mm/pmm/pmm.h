#pragma once
#include <basic_includes.h>

typedef struct {
    uint8_t* base;
    uint64_t free;
    uint64_t size;
} __attribute__((packed)) bitmap_stat_t;

void init_pmm();

uint64_t get_free_mem();
void update_bitmap_base(uint64_t offset);

void* pmalloc(size_t pages);
void pfree(void* pptr, size_t pages);