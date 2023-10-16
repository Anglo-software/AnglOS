#include "gdt.h"
#include "boot/cpu/cpu.h"
#include "threads/spinlock.h"
#include <basic_includes.h>

static gdt_desc_t gdt[NCPU_MAX][GDT_MAX_DESCRIPTORS];
static gdtr_t gdtr;
static uint16_t gindex[NCPU_MAX];
static spinlock lock = SPINLOCK_INITIALIZER;

void initGDT(uint64_t num_cpu) {
    spinLock(&lock);
    gdtr.limit = (sizeof(gdt_desc_t) * GDT_MAX_DESCRIPTORS) - 1;
    gdtr.base  = (uintptr_t)&gdt[num_cpu][0];

    gdtAddDescriptor(num_cpu, 0, 0, 0, 0);
    gdtAddDescriptor(num_cpu, 0, 0xFFFF,
                     GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_EXECUTABLE, 0);
    gdtAddDescriptor(num_cpu, 0, 0xFFFF, GDT_BASIC_DESCRIPTOR, 0);
    gdtAddDescriptor(num_cpu, 0, 0xFFFFF,
                     GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_EXECUTABLE,
                     GDT_GRANULARITY_4K | GDT_GRANULARITY_X32);
    gdtAddDescriptor(num_cpu, 0, 0xFFFFF, GDT_BASIC_DESCRIPTOR,
                     GDT_GRANULARITY_4K | GDT_GRANULARITY_X32);
    gdtAddDescriptor(num_cpu, 0, 0,
                     GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_EXECUTABLE,
                     GDT_GRANULARITY_X64);
    gdtAddDescriptor(num_cpu, 0, 0, GDT_BASIC_DESCRIPTOR, 0);
    gdtAddDescriptor(num_cpu, 0, 0, GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_DPL,
                     0);
    gdtAddDescriptor(num_cpu, 0, 0,
                     GDT_BASIC_DESCRIPTOR | GDT_DESCRIPTOR_DPL |
                         GDT_DESCRIPTOR_EXECUTABLE,
                     GDT_GRANULARITY_X64);

    gdtReload(&gdtr, GDT_OFFSET_KERNEL_CODE64, GDT_OFFSET_KERNEL_DATA64);
    spinUnlock(&lock);
}

void gdtAddDescriptor(uint64_t num_cpu, uint64_t base, uint32_t limit,
                      uint8_t access, uint8_t granularity) {
    if (gindex[num_cpu] >= GDT_MAX_DESCRIPTORS)
        return;

    gdt[num_cpu][gindex[num_cpu]].base_low  = base & 0xFFFF;
    gdt[num_cpu][gindex[num_cpu]].base_mid  = (base >> 16) & 0xFF;
    gdt[num_cpu][gindex[num_cpu]].base_high = (base >> 24) & 0xFF;

    gdt[num_cpu][gindex[num_cpu]].limit     = limit & 0xFFFF;

    gdt[num_cpu][gindex[num_cpu]].flags     = access;
    gdt[num_cpu][gindex[num_cpu]].granularity =
        granularity | ((limit >> 16) & 0x0F);

    gindex[num_cpu]++;
}

#define TSS_SIZE 0x70

uint16_t gdtInstallTSS(uint64_t tss, int num_cpu) {
    uint8_t tss_type = GDT_DESCRIPTOR_ACCESS | GDT_DESCRIPTOR_EXECUTABLE |
                       GDT_DESCRIPTOR_PRESENT;

    gdt_tss_desc_t* tss_desc = (gdt_tss_desc_t*)&gdt[num_cpu][gindex[num_cpu]];

    if (gindex[num_cpu] >= GDT_MAX_DESCRIPTORS)
        return 0;

    tss_desc->limit_0  = TSS_SIZE & 0xFFFF;
    tss_desc->addr_0   = tss & 0xFFFF;
    tss_desc->addr_1   = (tss & 0xFF0000) >> 16;
    tss_desc->type_0   = tss_type;
    tss_desc->limit_1  = (TSS_SIZE & 0xF0000) >> 16;
    tss_desc->addr_2   = (tss & 0xFF000000) >> 24;
    tss_desc->addr_3   = tss >> 32;
    tss_desc->reserved = 0;

    cpus[num_cpu].gdt  = &gdt[num_cpu][0];

    gindex[num_cpu] += 2;
    return (gindex[num_cpu] - 2) * GDT_DESCRIPTOR_SIZE;
}