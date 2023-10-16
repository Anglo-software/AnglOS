#include "pmm.h"
#include "boot/limine.h"
#include "libc/string.h"
#include "mm/paging/paging.h"

static volatile struct limine_memmap_request mmap_request = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0};

static uint64_t entry_count;
static struct limine_memmap_entry** mmap_entries;
uint64_t mem_size;           // In bytes
static uint64_t bitmap_size; // In bytes
static uint8_t* bitmap_base;
static uint64_t free_mem; // In bytes

#define NUM_BITMAPS 7
static bitmap_stat_t
    bitmaps[NUM_BITMAPS]; // 4K Blocks (1 page) to 256K Blocks (64 page)

static void pmmSetBit(uint8_t* base, uint64_t index)
{
    // Index is in BITS (The i-th block in this bitmap)
    uint8_t bit   = index % 8;
    uint64_t byte = index / 8;
    base[byte] |= 1UL << bit;
}

static void pmmClearBit(uint8_t* base, uint64_t index)
{
    // Index is in BITS (The i-th block in this bitmap)
    uint8_t bit   = index % 8;
    uint64_t byte = index / 8;
    base[byte] &= ~(1UL << bit);
}

static uint8_t pmmCheckBit(uint8_t* base, uint64_t index)
{
    // Index is in BITS (The i-th block in this bitmap)
    uint8_t bit   = index % 8;
    uint64_t byte = index / 8;
    return (base[byte] >> bit) & 1UL;
}

static uint8_t pmmGetStatusFromMMap(uint64_t page)
{
    for (uint64_t i = 0; i < entry_count; i++) {
        if (mmap_entries[i]->base / PAGE_SIZE <= page &&
            page <
                (mmap_entries[i]->base + mmap_entries[i]->length) / PAGE_SIZE) {
            return mmap_entries[i]->type;
        }
    }

    return LIMINE_MEMMAP_RESERVED;
}

uint64_t pmmGetFreeMem() { return free_mem; }

void pmmUpdateBitmapBase(uint64_t offset)
{
    bitmap_base += offset;
    for (int i = 0; i < NUM_BITMAPS; i++) {
        bitmaps[i].base += offset;
    }
}

#define L_CHILD(index) (index * 2)
#define R_CHILD(index) (index * 2 + 1)
#define PARENT(index)  (index / 2)

void initPMM()
{
    entry_count  = mmap_request.response->entry_count;
    mmap_entries = mmap_request.response->entries;
    mem_size     = mmap_entries[entry_count - 1]->base +
               mmap_entries[entry_count - 1]->length - 2UL * (1UL << 30);
    uint64_t num_pages = mem_size / PAGE_SIZE;

    uint64_t init_size = num_pages / 8;
    bitmap_size        = 0;
    for (int i = 0; i < NUM_BITMAPS; i++) {
        bitmaps[i].size = init_size >> i;
        bitmap_size += bitmaps[i].size;
    }

    size_t index = 0;
    while (index < entry_count) {
        if (mmap_entries[index]->length >= bitmap_size &&
            mmap_entries[index]->type == LIMINE_MEMMAP_USABLE) {
            break;
        }
        index++;
    }

    bitmap_base = (uint8_t*)mmap_entries[index]->base;

    mmap_entries[index]->base += bitmap_size;
    mmap_entries[index]->length -= bitmap_size;

    bitmaps[0].base = bitmap_base;
    bitmaps[0].free = 0;
    for (int i = 1; i < NUM_BITMAPS; i++) {
        bitmaps[i].base = bitmaps[i - 1].base + bitmaps[i - 1].size;
        bitmaps[i].free = 0;
    }

    for (int i = 0; i < NUM_BITMAPS; i++) {
        for (uint64_t j = 0; j < bitmaps[i].size; j++) {
            bitmaps[i].base[j] = 0xFF;
        }
    }

    for (uint64_t i = 0; i < bitmaps[0].size * 8; i++) {
        if (pmmGetStatusFromMMap(i) == LIMINE_MEMMAP_USABLE) {
            pmmClearBit(bitmaps[0].base, i);
            bitmaps[0].free++;
        }
    }

    for (int i = 1; i < NUM_BITMAPS; i++) {
        for (uint64_t j = 0; j < bitmaps[i].size * 8; j++) {
            if (!pmmCheckBit(bitmaps[i - 1].base, L_CHILD(j)) &&
                !pmmCheckBit(bitmaps[i - 1].base, R_CHILD(j))) {
                pmmClearBit(bitmaps[i].base, j);
                bitmaps[i].free++;
            }
        }
    }

    for (int i = 0; i < NUM_BITMAPS - 1; i++) {
        for (uint64_t j = 0; j < bitmaps[i].size * 8; j++) {
            if (!pmmCheckBit(bitmaps[i + 1].base, PARENT(j))) {
                pmmSetBit(bitmaps[i].base, j);
                bitmaps[i].free--;
            }
        }
    }

    free_mem = 0;
    for (int i = 0; i < NUM_BITMAPS; i++) {
        free_mem += bitmaps[i].free * (PAGE_SIZE << i);
    }
}

static void pmmPropagateDown(size_t pages, size_t level, uint64_t index)
{
    if (level == 0) {
        return;
    }
    else {
        if (pages == (1UL << level)) {
            return;
        }
        else if (pages <= (1UL << (level - 1))) {
            pmmPropagateDown(pages, level - 1, L_CHILD(index));
            pmmClearBit(bitmaps[level - 1].base, R_CHILD(index));
            bitmaps[level - 1].free++;
        }
        else {
            pmmPropagateDown(pages - (1 << (level - 1)), level - 1,
                             R_CHILD(index));
        }
    }
}

void* pmalloc(size_t pages)
{
    if (pages == 0) {
        return NULL;
    }

    size_t tmp = pages - 1;
    while (tmp & (tmp - 1)) {
        tmp = tmp & (tmp - 1);
    }
    tmp = tmp << 1;

    if ((1 << (NUM_BITMAPS - 1)) < tmp) {
        return NULL;
    }

    size_t index = 0;
    while (tmp >>= 1)
        index++;

    for (size_t i = index; i < NUM_BITMAPS; i++) {
        for (uint64_t j = 0; j < bitmaps[i].size * 8; j++) {
            if (!pmmCheckBit(bitmaps[i].base, j)) {
                pmmSetBit(bitmaps[i].base, j);
                bitmaps[i].free--;
                if (i != (NUM_BITMAPS - 1) &&
                    !pmmCheckBit(bitmaps[i + 1].base, PARENT(j))) {
                    pmmSetBit(bitmaps[i + 1].base, PARENT(j));
                    bitmaps[i + 1].free--;
                    pmmClearBit(bitmaps[i].base, j + 1);
                    bitmaps[i].free++;
                }
                pmmPropagateDown(pages, i, j);
                free_mem -= pages * PAGE_SIZE;
                return (void*)(j * (PAGE_SIZE << i));
            }
        }
    }

    return NULL;
}

static void coalesce(size_t pages, size_t level, uint64_t index)
{
    if (index % 2 == 0) {
        for (uint64_t i = index; i < index + pages; i += 2) {
            if (!pmmCheckBit(bitmaps[level].base, i) &&
                !pmmCheckBit(bitmaps[level].base, i + 1)) {
                pmmClearBit(bitmaps[level + 1].base, PARENT(i));
                bitmaps[level + 1].free++;
                pmmSetBit(bitmaps[level].base, i);
                pmmSetBit(bitmaps[level].base, i + 1);
                bitmaps[level].free -= 2;
            }
        }
        if (level + 1 != NUM_BITMAPS - 1) {
            coalesce(pages / 2, level + 1, PARENT(index));
        }
        return;
    }
    else {
        for (uint64_t i = index; i < index + pages; i += 2) {
            if (!pmmCheckBit(bitmaps[level].base, i - 1) &&
                !pmmCheckBit(bitmaps[level].base, i)) {
                pmmClearBit(bitmaps[level + 1].base, PARENT(i));
                bitmaps[level + 1].free++;
                pmmSetBit(bitmaps[level].base, i - 1);
                pmmSetBit(bitmaps[level].base, i);
                bitmaps[level].free -= 2;
            }
        }
        if (level + 1 != NUM_BITMAPS - 1) {
            coalesce(pages / 2, level + 1, PARENT(index));
        }
        return;
    }
}

void pfree(void* pptr, size_t pages)
{
    if (pptr == NULL || pages == 0) {
        return;
    }

    uint64_t index = (uint64_t)pptr / PAGE_SIZE;
    for (uint64_t i = index; i < index + pages; i++) {
        pmmClearBit(bitmaps[0].base, i);
        bitmaps[0].free++;
    }

    coalesce(pages, 0, index);

    free_mem += pages * PAGE_SIZE;
}