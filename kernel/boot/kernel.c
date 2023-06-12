#include <basic_includes.h>
#include "limine.h"
#include "fs/kmodule.h"
#include "drivers/vga/graphics.h"
#include "drivers/vga/vga_print.h"
#include "cpu/cpu.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
#include "drivers/acpi/acpi.h"
#include "drivers/apic/apic.h"
#include "mm/pmm/pmm.h"
#include "mm/paging/paging.h"
#include "mm/heap/memlib.h"
#include "mm/heap/heap.h"
#include "drivers/hpet/hpet.h"
#include "drivers/apic/timer.h"
#include "drivers/keyboard/keyboard.h"

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
    init_kmodules();
    init_sse();
    init_gdt();
    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }
    init_idt();
    init_acpi();
    init_apic();

    if (init_graphics(1024, 768, 32)) {
        really_done();
    }

    vga_printf("Initializing PMM       \n");
    init_pmm();
    vga_print("\033[1A");

    vga_printf("Initializing Paging    \n");
    init_paging();
    vga_print("\033[1A");

    vga_printf("Initializing Heap      \n");
    if (mem_init()) {
        really_done();
    }
    
    if (mm_init()) {
        really_done();
    }
    vga_print("\033[1A");

    vga_printf("Enabling Interrupts    \n");
    __asm__ volatile ("sti");
    vga_print("\033[1A");

    vga_printf("Initializing HPET      \n");
    init_hpet();
    vga_print("\033[1A");
    
    vga_printf("Initializing APIC Timer\n");
    init_timer();
    vga_print("\033[1A");

    vga_printf("Initializing Keyboard  \n");
    if(init_keyboard()) {
        really_done();
    }

    vga_print("\033[2J\033[H");
 
    // We're done, just hang...
    done();
}