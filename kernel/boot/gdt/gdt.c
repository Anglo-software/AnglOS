#include <basic_includes.h>
#include "gdt.h"

static gdt_desc_t gdt[GDT_MAX_DESCRIPTORS];

static gdtr_t gdtr;

static uint16_t gindex;

void init_gdt() {
    gdtr.limit = (sizeof(gdt_desc_t) * GDT_MAX_DESCRIPTORS) - 1;
    gdtr.base = (uintptr_t)&gdt[0];

    gdt_add_descriptor(0, 0,       0,                                                                     0);
    gdt_add_descriptor(0, 0xFFFF,  GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_EXECUTABLE,                      0);
    gdt_add_descriptor(0, 0xFFFF,  GDT_BASIC_DESCRIPTOR,                                                  0);
    gdt_add_descriptor(0, 0xFFFFF, GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_EXECUTABLE,                      GDT_GRANULARITY_4K | GDT_GRANULARITY_X32);
    gdt_add_descriptor(0, 0xFFFFF, GDT_BASIC_DESCRIPTOR,                                                  GDT_GRANULARITY_4K | GDT_GRANULARITY_X32);
    gdt_add_descriptor(0, 0,       GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_EXECUTABLE,                      GDT_GRANULARITY_X64);
    gdt_add_descriptor(0, 0,       GDT_BASIC_DESCRIPTOR,                                                  0);
    gdt_add_descriptor(0, 0,       GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_DPL,                             0);
    gdt_add_descriptor(0, 0,       GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_DPL | GDT_DESCRIPTOR_EXECUTABLE, GDT_GRANULARITY_X64);

    gdt_reload(&gdtr, GDT_OFFSET_KERNEL_CODE64, GDT_OFFSET_KERNEL_DATA64);
}

void gdt_add_descriptor(uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    if (gindex >= GDT_MAX_DESCRIPTORS) 
        return;

    gdt[gindex].base_low = base & 0xFFFF;
    gdt[gindex].base_mid = (base >> 16) & 0xFF;
    gdt[gindex].base_high = (base >> 24) & 0xFF;

    gdt[gindex].limit = limit & 0xFFFF;

    gdt[gindex].flags = access;
    gdt[gindex].granularity = granularity | ((limit >> 16) & 0x0F);

    gindex++;
}

#define TSS_SIZE 0x70

uint16_t gdt_install_tss(uint64_t tss) {
    uint8_t tss_type = GDT_DESCRIPTOR_ACCESS | GDT_DESCRIPTOR_EXECUTABLE | GDT_DESCRIPTOR_PRESENT;

    gdt_tss_desc_t* tss_desc = (gdt_tss_desc_t *)&gdt[gindex];

    if (gindex >= GDT_MAX_DESCRIPTORS)
        return 0;

    tss_desc->limit_0 = TSS_SIZE & 0xFFFF;
    tss_desc->addr_0 = tss & 0xFFFF;
    tss_desc->addr_1 = (tss & 0xFF0000) >> 16;
    tss_desc->type_0 = tss_type;
    tss_desc->limit_1 = (TSS_SIZE & 0xF0000) >> 16;
    tss_desc->addr_2 = (tss & 0xFF000000) >> 24;
    tss_desc->addr_3 = tss >> 32;
    tss_desc->reserved = 0;

    gindex += 2;
    return (gindex - 2) * GDT_DESCRIPTOR_SIZE;
}
