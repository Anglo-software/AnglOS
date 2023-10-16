#include "syscall.h"
#include "boot/cpu/cpu.h"
#include "boot/tss/tss.h"
#include "device/apic/apic.h"
#include "device/io.h"
#include "libc/stdio.h"
#include "mm/heap/heap.h"
#include "threads/spinlock.h"
#include "threads/thread.h"

extern void syscallEntryPoint();
static spinlock lock = SPINLOCK_INITIALIZER;

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
    hi = 0x00300008;
    wrmsr(0xC0000081, lo, hi);

    uint64_t cpu_id = lapicReadReg(APIC_APICID) >> 24;

    spinLock(&lock);
    gs_base_t* gs = (gs_base_t*)kcalloc(sizeof(gs_base_t));
    ctx_t* ctx    = (ctx_t*)kcalloc(sizeof(ctx_t));
    spinUnlock(&lock);
    gs->kernel  = 1;
    gs->ker_rsp = (uint64_t)tssGetStack(cpu_id, 0);
    gs->cpu     = (cpu_t*)&cpus[cpu_id];
    gs->ctx     = ctx;
    lo          = (uint32_t)((uint64_t)gs & 0xFFFFFFFF);
    hi          = (uint32_t)((uint64_t)gs >> 32);
    wrmsr(0xC0000102, (uint32_t)lo, (uint32_t)hi);
}

int syscalltest(int num) {
    kprintf("Syscall called: %d\n", num);
    return 3;
}

int syscalltest2(int num1, int num2) {
    kprintf("The sum of %d and %d is %d\n", num1, num2, num1 + num2);
    return 0;
}

int syscalltest3(int n1, int n2, int n3, int n4, int n5, int n6) {
    kprintf("%d, %d, %d, %d, %d, %d\n", n1, n2, n3, n4, n5, n6);
}