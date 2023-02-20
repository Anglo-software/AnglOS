#include <boot/gdt.h>

extern void gdt_flush(uint32_t);

// Internal function prototypes.
static void gdt_set_gate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);

gdt_seg_t gdt_entries[5];
gdt_ptr_t gdt_ptr;

static void init_gdt() {
    gdt_ptr.size = (sizeof(gdt_seg_t) * 5) - 1;
    gdt_ptr.offset = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                         // Null segment
    gdt_set_gate(1, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
    gdt_set_gate(1, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
    gdt_set_gate(1, 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
    gdt_set_gate(1, 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment
}

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].limit_high  = (limit >> 16) & 0x0F;
    gdt_entries[num].flags       = flags;
    gdt_entries[num].access      = access;
}

static void gdt_set_sys_gate(int32_t num, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].limit_high  = (limit >> 16) & 0x0F;
    gdt_entries[num].flags       = flags;
    gdt_entries[num].access      = access;
    gdt_entries[num].base_extra  = (base >> 32) & 0xFFFFFFFF;
    gdt_entries[num].reserved    = 0x00000000;
}