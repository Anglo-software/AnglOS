#include "isr.h"
#include "../tss/tss.h"
#include "device/apic/apic.h"
#include "idt.h"
#include "libc/stdio.h"

uint64_t interrupt_handlers[256];

extern uint64_t isr_stub_table[];

void isrInstall() {
    for (int vector = 0; vector < 32; vector++) {
        idtSetDescriptor(vector, isr_stub_table[vector],
                         IDT_DESCRIPTOR_EXCEPTION, TSS_IST_EXCEPTION);
    }
}

/* To print the message which defines every exception */
char* exception_messages[] = {"Division By Zero",
                              "Debug",
                              "Non Maskable Interrupt",
                              "Breakpoint",
                              "Into Detected Overflow",
                              "Out of Bounds",
                              "Invalid Opcode",
                              "No Coprocessor",

                              "Double Fault",
                              "Coprocessor Segment Overrun",
                              "Bad TSS",
                              "Segment Not Present",
                              "Stack Fault",
                              "General Protection Fault",
                              "Page Fault",
                              "Unknown Interrupt",

                              "Coprocessor Fault",
                              "Alignment Check",
                              "Machine Check",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",

                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved",
                              "Reserved"};

extern isrPageFaultHandler(isr_frame_t* r);

void isrHandler(isr_frame_t* r) {
    if (r->base.vector == 0xE) {
        isrPageFaultHandler(r);
    }
    else {
        kprintf("received interrupt: %ld from CPU %d\n", r->base.vector,
                lapicReadReg(APIC_APICID) >> 24);
        kprintf("[%s] with error code: %lx @ %lx\n",
                exception_messages[r->base.vector], r->base.error_code,
                r->base.rip);
        __asm__ volatile("cli; hlt");
    }
}

void irqRegisterHandler(uint8_t n, void* handler) {
    interrupt_handlers[n] = (uint64_t)handler;
    idtSetDescriptor(n, isr_stub_table[n], IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
}