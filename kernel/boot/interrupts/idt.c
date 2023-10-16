#include "../interrupts/idt.h"
#include "../gdt/gdt.h"
#include "../tss/tss.h"
#include "isr.h"

static __attribute__((aligned(4096))) idt_desc_t idt[IDT_MAX_DESCRIPTORS];

static idtr_t idtr;

void idtSetDescriptor(uint8_t vector, uintptr_t isr, uint8_t flags,
                      uint8_t ist) {
    idt_desc_t* descriptor = &idt[vector];

    descriptor->base_low   = isr & 0xFFFF;
    descriptor->cs         = GDT_OFFSET_KERNEL_CODE64;
    descriptor->ist        = ist;
    descriptor->attributes = flags;
    descriptor->base_mid   = (isr >> 16) & 0xFFFF;
    descriptor->base_high  = (isr >> 32) & 0xFFFFFFFF;
    descriptor->rsv0       = 0;
}

void initIDT() {
    idtr.base  = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_desc_t) * IDT_MAX_DESCRIPTORS - 1;

    isrInstall();

    idtReload(&idtr);
}

void idtAPReload() {
    __asm__ volatile("mov rax, %0;"
                     "lidt [rax]"
                     :
                     : "r"(&idtr)
                     : "memory");
}