#include <basic_includes.h>
#include "limine.h"
#include "libc/string.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
#include "terminal/terminal.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/8529/pic.h"


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
 
    if (init_terminal()) {
        really_done();
    }

    __asm__ volatile ("sti");

    if(init_keyboard()) {
        print("Keyboard could not be initialized, halting...\n");
        really_done();
    }

    for (int i = 0; i < 22*47; i++) {
        if (i / 22 == 23 && i % 22 == 0) {
            print("                   Welcome to:\n");
        }
        else if (i / 22 == 25 && i % 22 == 0) {
            print("                   Unnamed-OS\n");
        }
        else if (i % 22 == 0 && i != 0) {
            print("\n");
        }
        print("\033[38;2;255;255;255m");
        print("\033[48;2;90;90;90m");
        if (i < 0x10) {
            printi(0, 1, 16);
            printi(0, 1, 16);
        }
        else if (i < 0x100) {
            printi(0, 1, 16);
        }
        printi(i, 5, 16);
        print("\033[0m");
    }
 
    // We're done, just hang...
    done();
}