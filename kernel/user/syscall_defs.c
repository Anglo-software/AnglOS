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
    hi = 0x00300028;
    wrmsr(0xC0000081, lo, hi);

    uint64_t cpu_id = lapicReadReg(APIC_APICID) >> 24;

    spinLock(&lock);
    gs_base_t* gs = (gs_base_t*)kcalloc(sizeof(gs_base_t));
    ctx_t* ctx    = (ctx_t*)kcalloc(sizeof(ctx_t));
    spinUnlock(&lock);
    gs->kernel  = 0;
    gs->ker_rsp = 0;
    gs->cpu     = (cpu_t*)&cpus[cpu_id];
    gs->ctx     = ctx;
    lo          = (uint32_t)((uint64_t)gs & 0xFFFFFFFF);
    hi          = (uint32_t)((uint64_t)gs >> 32);
    wrmsr(0xC0000102, (uint32_t)lo, (uint32_t)hi);
    __asm__ volatile("swapgs");

    spinLock(&lock);
    gs  = (gs_base_t*)kcalloc(sizeof(gs_base_t));
    ctx = (ctx_t*)kcalloc(sizeof(ctx_t));
    spinUnlock(&lock);
    gs->kernel  = 0;
    gs->ker_rsp = (uint64_t)tssGetStack(cpu_id, 0);
    gs->cpu     = (cpu_t*)&cpus[cpu_id];
    gs->ctx     = ctx;
    lo          = (uint32_t)((uint64_t)gs & 0xFFFFFFFF);
    hi          = (uint32_t)((uint64_t)gs >> 32);
    wrmsr(0xC0000102, (uint32_t)lo, (uint32_t)hi);
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