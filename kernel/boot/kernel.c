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
#include "threads/thread.h"
#include "tss/tss.h"
#include "user/elf.h"
#include "user/syscall_defs.h"
#include <basic_includes.h>

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST, .revision = 0, .stack_size = 1024 * 1024};

static void mainLoop();
static void _start_ap();

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
    initSyscall();

    cpuSTI();

    initHPET();
    initAPICTimer();

    cpuCLI();

    initPCI();
    initInputQueue();
    initConsole();
    initKeyboard();
    initThread();

    // for (int i = 1; i < num_cpus; i++) {
    //     smpStartAP((uint64_t)_start_ap, i);
    // }

    cpuSTI();

    // initNVMe();

    // mainLoop();

    ioapic_redirection_t redir = {.destination_mode = 0, .destination = 0};
    keyboardSetRedir(&redir);

    kmodule_t* prog_file   = kmoduleFindByPath("/resources/testprog.elf");
    thread_t* thread = threadCreate(prog_file->address, PRIO_DEFAULT, 0);
    threadContextSwitch(thread);

    cpuCLI();
    cpuHLT();
}

static void _start_ap(struct limine_smp_info* info) {
    cpu_t* cpu = &cpus[info->processor_id];

    cpuCLI();
    initSSE();
    initAPPaging();
    initGDT(cpu->cpu_id);
    initTSS(cpu->cpu_id);
    idtAPReload();
    initAPAPIC();
    initSyscall();
    initAPAPICTimer();
    cpuSTI();

    while (true) {
        cpuHLT();
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
                cpuHLT();
                kprintf("%s", "Exiting");
                cpuHLT();
                kprintf(".");
                cpuHLT();
                kprintf(".");
                cpuHLT();
                kprintf(".");
                cpuHLT();
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