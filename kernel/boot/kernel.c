#include <basic_includes.h>
#include "limine.h"
#include "libc/string.h"
#include "terminal/terminal.h"
#include "gdt/gdt.h"
#include "tss/tss.h"
#include "interrupts/idt.h"
#include "mm/pmm/pmm.h"
#include "mm/paging/paging.h"
#include "drivers/vga/graphics.h"
#include "drivers/vga/text.h"
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

static uint64_t wait(uint64_t time) {
    volatile uint64_t sum = 0;
    for (uint64_t i = 0; i < time; i++) {
        sum += i;
    }
    return sum;
}

void _start(void) {
    if (stack_request.response == NULL) {
        really_done();
    }

    init_terminal();
    
    init_gdt();
    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }
    init_idt();

    init_pmm();
    init_paging();

    if (init_graphics(1024, 768, 32)) {
        print("VGA could not be initialized, halting...\n");
        really_done();
    }

    for (uint16_t i = 0; i < 256; i++) {
        draw_char_at_cursor((uint8_t)i, false);
        advance_cursor();
        wait(100000);
    }

    __asm__ volatile ("sti");

    if(init_keyboard()) {
        print("Keyboard could not be initialized, halting...\n");
        really_done();
    } 
 
    // We're done, just hang...
    done();
}