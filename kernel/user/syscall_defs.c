#include "syscall_defs.h"
#include "boot/cpu/cpu.h"
#include "boot/tss/tss.h"
#include "device/apic/apic.h"
#include "device/input/input.h"
#include "device/io.h"
#include "libc/stdio.h"
#include "mm/heap/heap.h"
#include "threads/spinlock.h"
#include "threads/thread.h"

extern void syscallEntryPoint();

void initSyscall() {
    uint64_t syscall_entry_addr = (uint64_t)&syscallEntryPoint;
    uint32_t lo, hi;
    lo = syscall_entry_addr & 0xFFFFFFFF;
    hi = (syscall_entry_addr >> 32) & 0xFFFFFFFF;
    wrmsr(0xC0000082, lo, hi);
    rdmsr(0xC0000080, &lo, &hi);
    lo = lo | 1;
    wrmsr(0xC0000080, lo, hi);
    rdmsr(0xC0000081, &lo, &hi);
    hi = 0x00300028;
    wrmsr(0xC0000081, lo, hi);

    setUserGS(0);
    setKernGS(0);
}

int sys_halt() {
    cpuHLT();
    return 0;
}

int sys_print(char* str, size_t len) {
    consolePutbuf(str, len);
    return 0;
}

char sys_getc() { return inputGetc(); }

void sys_exit(int code) { return; }