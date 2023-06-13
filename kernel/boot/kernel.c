#include <basic_includes.h>
#include "limine.h"
#include "fs/kmodule.h"
#include "device/vga/graphics.h"
#include "cpu/cpu.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
#include "device/acpi/acpi.h"
#include "device/apic/apic.h"
#include "mm/pmm/pmm.h"
#include "mm/paging/paging.h"
#include "mm/heap/memlib.h"
#include "mm/heap/heap.h"
#include "device/hpet/hpet.h"
#include "device/apic/timer.h"
#include "device/keyboard/keyboard.h"
#include "libc/stdio.h"

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
    printf("Halting\n");
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
    init_pmm();
    init_paging();
    if (mem_init()) {
        really_done();
    }
    
    if (kmeminit()) {
        really_done();
    }
    __asm__ volatile ("sti");
    init_hpet();
    init_timer();
    if(init_keyboard()) {
        really_done();
    }

    printf("Welcome to Angl-OS\n");
 
    // We're done, just hang...
    done();
}