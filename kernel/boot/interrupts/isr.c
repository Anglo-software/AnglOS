#include "isr.h"
#include "../tss/tss.h"
#include "device/apic/apic.h"
#include "idt.h"
#include "libc/stdio.h"

isr_t interrupt_handlers[256];

void isrInstall() {
    idtSetDescriptor(0, (uint64_t)isr0, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(1, (uint64_t)isr1, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(2, (uint64_t)isr2, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(3, (uint64_t)isr3, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(4, (uint64_t)isr4, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(5, (uint64_t)isr5, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(6, (uint64_t)isr6, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(7, (uint64_t)isr7, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(8, (uint64_t)isr8, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(9, (uint64_t)isr9, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(10, (uint64_t)isr10, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(11, (uint64_t)isr11, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(12, (uint64_t)isr12, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(13, (uint64_t)isr13, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(14, (uint64_t)isr14, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(15, (uint64_t)isr15, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(16, (uint64_t)isr16, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(17, (uint64_t)isr17, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(18, (uint64_t)isr18, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(19, (uint64_t)isr19, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(20, (uint64_t)isr20, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(21, (uint64_t)isr21, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(22, (uint64_t)isr22, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(23, (uint64_t)isr23, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(24, (uint64_t)isr24, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(25, (uint64_t)isr25, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(26, (uint64_t)isr26, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(27, (uint64_t)isr27, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(28, (uint64_t)isr28, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(29, (uint64_t)isr29, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(30, (uint64_t)isr30, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);
    idtSetDescriptor(31, (uint64_t)isr31, IDT_DESCRIPTOR_EXCEPTION,
                     TSS_IST_EXCEPTION);

    idtSetDescriptor(32, (uint64_t)irq0, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(33, (uint64_t)irq1, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(34, (uint64_t)irq2, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(35, (uint64_t)irq3, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(36, (uint64_t)irq4, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(37, (uint64_t)irq5, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(38, (uint64_t)irq6, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(39, (uint64_t)irq7, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(40, (uint64_t)irq8, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(41, (uint64_t)irq9, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(42, (uint64_t)irq10, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(43, (uint64_t)irq11, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(44, (uint64_t)irq12, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(45, (uint64_t)irq13, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(46, (uint64_t)irq14, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
    idtSetDescriptor(47, (uint64_t)irq15, IDT_DESCRIPTOR_EXTERNAL,
                     TSS_IST_ROUTINE);
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

void isrHandler(registers_t* r) {
    kprintf("received interrupt: %ld from CPU %d\n", r->vector,
            lapicReadReg(APIC_APICID) >> 24);
    kprintf("[%s] with error code: %lx @ %lx\n", exception_messages[r->vector],
            r->error_code, r->rip);
    if (r->vector == 0xE) {
        kprintf("Culprit address: %lx\n", r->cr2);
    }
    __asm__ volatile("cli; hlt");
}

void isrRegisterHandler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irqHandler(registers_t* r) {
    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r->vector] != 0) {
        isr_t handler = interrupt_handlers[r->vector];
        handler(r);
    }
    else {
        apicSendEOI();
    }
}