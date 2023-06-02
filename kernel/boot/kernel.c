#include <basic_includes.h>
#include "limine.h"
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

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
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

    if (init_graphics(1024, 768, 32)) {
        really_done();
    }

    vga_printf("Initializing SSE       \n");
    init_sse();
    vga_print("\033[1A");
    
    vga_printf("Initializing GDT       \n");
    init_gdt();
    vga_print("\033[1A");

    vga_printf("Initializing TSS       \n");
    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }
    vga_print("\033[1A");

    vga_printf("Initializing IDT       \n");
    init_idt();
    vga_print("\033[1A");

    vga_printf("Initializing ACPI      \n");
    init_acpi();
    vga_print("\033[1A");

    vga_printf("Initializing APIC      \n");
    init_apic();
    vga_print("\033[1A");

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

    struct limine_file* file = module_request.response->modules[0];
    vga_printf("%s\n", (char*)(file->address));
 
    // We're done, just hang...
    done();
}