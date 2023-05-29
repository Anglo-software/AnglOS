#include <basic_includes.h>
#include "limine.h"
#include "cpu/cpu.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
#include "drivers/acpi/acpi.h"
#include "drivers/apic/apic.h"
#include "mm/pmm/pmm.h"
#include "mm/paging/paging.h"
#include "drivers/vga/graphics.h"
#include "drivers/vga/vga_print.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/hpet/hpet.h"

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 1024*1024
};
 
static void done(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void really_done(void) {
    vga_print("Halting\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void _start(void) {
    if (stack_request.response == NULL) {
        really_done();
    }

    init_sse();
    
    init_gdt();
    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }
    init_idt();

    init_acpi();
    init_apic();
    init_hpet();

    init_pmm();
    init_paging();

    if (init_graphics(1024, 768, 32)) {
        really_done();
    }

    __asm__ volatile ("sti");

    if(init_keyboard()) {
        really_done();
    }
 
    // We're done, just hang...
    done();
}