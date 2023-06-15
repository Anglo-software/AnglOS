#include <basic_includes.h>
#include "limine.h"
#include "fs/kmodule.h"
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
#include "device/vga/graphics.h"
#include "device/pci/pci.h"
#include "device/input/input.h"
#include "device/keyboard/keyboard.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "device/io.h"

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 1024*1024
};

static void main_loop();

void _start() {
    init_sse();
    init_gdt();

    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }

    init_idt();
    init_acpi();
    init_apic();
    init_pmm();
    init_paging();
    mem_init();
    kmeminit();

    sti();

    init_hpet();
    init_timer();
    init_kmodules();
    init_graphics(1024, 768, 32);

    init_pci();
    init_input();
    init_keyboard();

    main_loop();
}

#define line_len 128
char linebuf[line_len + 1];
unsigned num = 0;
 
static void main_loop() {
    printf("> ");
    for (;;) {
        uint8_t c = input_getc();
        if (c == '\n') {
            printf("\n");
            printf("%s\n", linebuf);
            printf("> ");
            num = 0;
            memset(linebuf, '\0', line_len + 1);
            continue;
        }
        else if (c == '\b') {
            if (num != 0) {
                linebuf[num] = '\0';
                num--;
                printf("\b");
            }
            continue;
        }
        if (num + 1 == line_len + 1) {
            continue;
        }
        linebuf[num] = c;
        num++;
        printf("%c", c);
    }
}