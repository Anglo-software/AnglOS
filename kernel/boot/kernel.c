#include <basic_includes.h>
#include "limine.h"
#include "libc/string.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
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

    init_gdt();
    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }
    init_idt();

    struct limine_terminal* terminal = init_terminal();
 
    if (!terminal) {
        really_done();
    }

    __asm__ volatile ("sti");

    if(init_keyboard()) {
        print("Keyboard could not be initialized, halting...\n");
        really_done();
    }
 
    // We're done, just hang...
    done();
}