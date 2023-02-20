#include "gdt.h"

extern void setGDT(uint64_t);
extern void reloadSegments();

static gdt_seg_t gdt_entries[] = {
    {0},
    {
        .limit       = 0xffff,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b10011010,
        .granularity = 0b00000000,
        .base_high   = 0x00
    },
    {
        .limit       = 0xffff,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b10010010,
        .granularity = 0b00000000,
        .base_high   = 0x00
    },
    {
        .limit       = 0xffff,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b10011010,
        .granularity = 0b11001111,
        .base_high   = 0x00
    },
    {
        .limit       = 0xffff,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b10010010,
        .granularity = 0b11001111,
        .base_high   = 0x00
    },
    {
        .limit       = 0x0000,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b10011010,
        .granularity = 0b00100000,
        .base_high   = 0x00
    },
    {
        .limit       = 0x0000,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b10010010,
        .granularity = 0b00000000,
        .base_high   = 0x00
    },
    {
        .limit       = 0x0000,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b11111010,
        .granularity = 0b00100000,
        .base_high   = 0x00
    },
    {
        .limit       = 0x0000,
        .base_low    = 0x0000,
        .base_middle = 0x00,
        .access      = 0b11110010,
        .granularity = 0b00000000,
        .base_high   = 0x00
    }
};

gdt_ptr_t gdt_ptr = {.limit = sizeof(gdt_entries) - 1, .ptr = (uint64_t)gdt_entries};

void init_gdt() {
    setGDT((uint64_t)&gdt_ptr);
    reloadSegments();
}