#pragma once
#include <basic_includes.h>

typedef struct {
    uint8_t* base;
    uint64_t free;
    uint64_t size;
} __attribute__((packed)) bitmap_stat_t;

void initPMM();

uint64_t pmmGetFreeMem();
void pmmUpdateBitmapBase(uint64_t offset);

void* pmalloc(size_t pages);
void pfree(void* pptr, size_t pages);