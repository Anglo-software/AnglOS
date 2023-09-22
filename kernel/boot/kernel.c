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
#include "user/syscall.h"

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 1024*1024
};

static void main_loop();
static void userfunc();

NO_RETURN
void _start() {
    init_sse();
    init_syscall();
    init_gdt();
    init_pmm();

    init_kmodules();
    init_graphics(1024, 768, 32);

    for (int i = 0; i < TSS_MAX_CPUS; i++) {
        init_tss(i);
    }

    init_idt();
    init_paging();
    mem_init();
    kmeminit();
    init_acpi();
    init_apic();

    sti();

    init_hpet();
    init_timer();

    cli();

    init_pci();
    init_input();
    init_keyboard();

    sti();

    main_loop();
}

unsigned size = 16;
unsigned num = 0;

extern uint64_t mem_size;

NO_RETURN
static void main_loop() {
    // char* linebuf = kcalloc(size + 1);
    // printf("> ");
    // timer_periodic_start(500000000);
    // for (;;) {
    //     uint8_t c = input_getc();
    //     if (c == '\n') {
    //         printf("\n");
    //         if (!strncmp(linebuf, "q", 1)) {
    //             __asm__ volatile ("hlt");
    //             printf("%s", "Exiting");
    //             __asm__ volatile ("hlt");
    //             printf(".");
    //             __asm__ volatile ("hlt");
    //             printf(".");
    //             __asm__ volatile ("hlt");
    //             printf(".");
    //             __asm__ volatile ("hlt");
    //             outw(0x604, 0x2000);
    //         }
    //         else {
    //             printf("%s\n", linebuf);
    //         }
    //         printf("> ");
    //         num = 0;
    //         memset(linebuf, '\0', size + 1);
    //         continue;
    //     }
    //     else if (c == '\b') {
    //         if (num != 0) {
    //             linebuf[num] = '\0';
    //             num--;
    //             printf("\b");
    //         }
    //         continue;
    //     }
    //     if (num == size) {
    //         linebuf = krealloc(linebuf, size + 16);
    //         size += 16;
    //     }
    //     if (c != CURSOR_EN && c != CURSOR_DS) {
    //         linebuf[num] = c;
    //         num++;
    //     }
    //     printf("%c", c);
    //     if (c == SCREEN_CLEAR) {
    //         printf("> ");
    //         num = 0;
    //         memset(linebuf, '\0', size + 1);
    //         continue;
    //     }
    // }

    void* vcodeptr = 0x0000000080000000;
    void* vstackptr = 0x0000000080010000 - PAGE_SIZE;
    void* ptr = vmalloc(vcodeptr, 1, PAGE_FLAG_USERSUPER | PAGE_FLAG_READWRITE | PAGE_FLAG_PRESENT);
    void* usrptr = (void*)&userfunc;

    memcpy(ptr, usrptr, PAGE_SIZE);

    void* stack = vmalloc(vstackptr, 1, PAGE_FLAG_USERSUPER | PAGE_FLAG_READWRITE | PAGE_FLAG_PRESENT);

    __asm__ volatile (".intel_syntax noprefix;"
                      "mov rsp, 0x80010000;"
                      "mov ecx, 0x80000000;"
                      "mov r11, 0x202;"
                      ".att_syntax prefix;");
    __asm__ volatile ("sysretq;");

    cli();
    halt();
}

__attribute__((noreturn, aligned(4096)))
void userfunc() {
    volatile bool a = false;
    while(true) {
        if (a == false) {
            a = true;
        }
        else {
            a = false;
        }
    }

    bool b = a;
}