#include "cpu/cpu.h"
#include "cpu/smp.h"
#include "device/acpi/acpi.h"
#include "device/apic/apic.h"
#include "device/apic/timer.h"
#include "device/hpet/hpet.h"
#include "device/input/input.h"
#include "device/io.h"
#include "device/keyboard/keyboard.h"
#include "device/nvme/nvme.h"
#include "device/pci/pci.h"
#include "device/vga/graphics.h"
#include "fs/kmodule.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "libc/kernel/console.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "limine.h"
#include "mm/heap/heap.h"
#include "mm/heap/memlib.h"
#include "mm/paging/paging.h"
#include "mm/pmm/pmm.h"
#include "threads/spinlock.h"
#include "tss/tss.h"
#include "user/syscall.h"
#include "user/userentry.h"
#include <basic_includes.h>
#include <stdatomic.h>

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST, .revision = 0, .stack_size = 1024 * 1024};

static void mainLoop();
static void _start_ap();
static void gotoUser();

NO_RETURN
void _start() {
    initSSE();
    initGDT(0);
    initPMM();

    initKmodules();
    initGraphics(1024, 768, 32);

    initSMP();

    initTSS(0);

    initIDT();
    initPaging();
    initKernelHeapMem();
    initKernelHeap();
    initACPI();
    initAPIC();

    sti();

    initHPET();
    initAPICTimer();

    cli();

    initPCI();
    initInputQueue();
    initConsole();
    initKeyboard();
    initSyscall();

    for (int i = 1; i < num_cpus; i++) {
        smpStartAP((uint64_t)_start_ap, i);
    }

    sti();

    // initNVMe();

    // mainLoop();

    gotoUser();

    cli();
    halt();
}

static void _start_ap(struct limine_smp_info* info) {
    cpu_t* cpu = &cpus[info->processor_id];

    cli();
    initSSE();
    initAPPaging();
    initGDT(cpu->cpu_id);
    initTSS(cpu->cpu_id);
    idtAPReload();
    initAPAPIC();
    initAPAPICTimer();
    initSyscall();
    sti();

    while (true) {
        halt();
    }
}

unsigned size = 16;
unsigned num  = 0;

extern uint64_t mem_size;

NO_RETURN
static void mainLoop() {
    ioapic_redirection_t redir = {.destination_mode = 0, .destination = 0};
    keyboardSetRedir(&redir);
    char* linebuf = kcalloc(size + 1);
    kprintf("> ");
    apicTimerPeriodicStart(500000000);
    for (;;) {
        uint8_t c = inputGetc();
        if (c == '\n') {
            kprintf("\n");
            if (!strncmp(linebuf, "q", 1)) {
                halt();
                kprintf("%s", "Exiting");
                halt();
                kprintf(".");
                halt();
                kprintf(".");
                halt();
                kprintf(".");
                halt();
                outw(0x604, 0x2000);
            }
            else {
                kprintf("%s\n", linebuf);
            }
            kprintf("> ");
            num = 0;
            memset(linebuf, '\0', size + 1);
            continue;
        }
        else if (c == '\b') {
            if (num != 0) {
                linebuf[num] = '\0';
                num--;
                kprintf("\b");
            }
            continue;
        }
        if (num == size) {
            linebuf = krealloc(linebuf, size + 16);
            size += 16;
        }
        if (c != CURSOR_EN && c != CURSOR_DS) {
            linebuf[num] = c;
            num++;
        }
        kprintf("%c", c);
        if (c == SCREEN_CLEAR) {
            kprintf("> ");
            num = 0;
            memset(linebuf, '\0', size + 1);
            continue;
        }
    }
}

void gotoUser() {
    void* vcodeptr  = (void*)0x0000000000010000;
    void* vstackptr = (void*)0x00007FFFFFFFF000 - PAGE_SIZE;
    void* ptr =
        vmalloc(vcodeptr, 1,
                PAGE_FLAG_USERSUPER | PAGE_FLAG_READWRITE | PAGE_FLAG_PRESENT);
    void* usrptr = (void*)&kerneluserentry;

    memcpy(ptr, usrptr, PAGE_SIZE);

    void* stack =
        vmalloc(vstackptr, 1,
                PAGE_FLAG_USERSUPER | PAGE_FLAG_READWRITE | PAGE_FLAG_PRESENT);

    __asm__ volatile(".intel_syntax noprefix;"
                     "mov rsp, %0;"
                     "mov rcx, %1;"
                     "mov r11, 0x202;"
                     ".att_syntax prefix;"
                     :
                     : "r"((uint64_t)vstackptr + PAGE_SIZE),
                       "r"((uint64_t)vcodeptr));
    __asm__ volatile("sysretq;");
}