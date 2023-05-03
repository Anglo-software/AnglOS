#include <basic_includes.h>
#include "limine.h"
#include "libc/string.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
#include "mm/pmm/pmm.h"
#include "terminal/terminal.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/8259/pic.h"

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
    print("Halting\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void _start(void) {

    if (stack_request.response == NULL) {
        really_done();
    }

    struct limine_terminal* terminal = init_terminal();
 
    if (!terminal) {
        really_done();
    }

    init_gdt();
    print("Initialized GDT\n");

    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }
    print("Initialized TSS\n");

    init_idt();
    print("Initialized IDT\n");

    init_pmm();
    // print("Initialized PMM\n");

    __asm__ volatile ("sti");

    if(init_keyboard()) {
        print("Keyboard could not be initialized, halting...\n");
        really_done();
    }
    // print("Initialized keyboard\n");
 
    // We're done, just hang...
    done();
}