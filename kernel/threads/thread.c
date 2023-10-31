#include "thread.h"
#include "boot/cpu/cpu.h"
#include "boot/tss/tss.h"
#include "device/io.h"
#include "libc/stdio.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"
#include "user/elf.h"
#include <stdatomic.h>

static struct list threadlist[NCPU_MAX];
static atomic_ulong next_thread_id;
extern void* page_direct_base;

void initThread() {
    for (int i = 0; i < ncpu; i++) {
        list_init(&threadlist[i]);
    }
}

thread_t* threadCreate(void* elf, uint64_t priority, uint64_t parent) {
    uint64_t cpuid         = next_thread_id % ncpu;
    thread_t* thread       = (thread_t*)kmalloc(sizeof(thread_t));
    void* cr3              = pagingCreateUser();
    address_space_t* space = addressSpaceNew((uint64_t)cr3);
    uint64_t oldcr3 = (uint64_t)pagingGetCR3() - (uint64_t)page_direct_base;
    pagingSetCR3((void*)((uint64_t)cr3 - (uint64_t)page_direct_base));
    uint64_t entry         = elfLoad(space, elf);
    pagingSetCR3((void*)oldcr3);

    thread->space          = space;
    thread->priority       = priority;
    thread->parent         = parent;
    thread->id             = next_thread_id++;
    thread->magic          = THREAD_MAGIC;
    thread->state          = THREAD_READY;
    thread->stack          = (void*)STACK_START;
    list_push_back(&threadlist[cpuid], &thread->elem);

    addressSpaceAddEntry(space, ADDRSPACE_TYPE_STACK,
                         ADDRSPACE_FLAG_READABLE | ADDRSPACE_FLAG_USER |
                             ADDRSPACE_FLAG_WRITEABLE,
                         2 * PAGE_SIZE, STACK_START - PAGE_SIZE);

    thread->gs           = (gs_base_t*)kmalloc(sizeof(gs_base_t));
    thread->gs->cpu      = (cpu_t*)&cpus[cpuid];
    thread->gs->kernel   = 0;
    thread->gs->pc       = entry;
    thread->gs->thread   = (void*)thread;
    thread->gs->usr_rsp  = thread->stack;
    thread->gs->ker_rsp  = (uint64_t)tssGetStack(cpuid, 0);

    thread->gs->ctx      = (ctx_t*)kmalloc(sizeof(ctx_t));
    thread->gs->ctx->cr3 = (void*)((uint64_t)cr3 - (uint64_t)page_direct_base);
    thread->gs->ctx->rip = entry;
    thread->gs->ctx->registers.rax = 0;
    thread->gs->ctx->registers.rbx = 0;
    thread->gs->ctx->registers.rcx = 0;
    thread->gs->ctx->registers.rdx = 0;
    thread->gs->ctx->registers.rdi = 0;
    thread->gs->ctx->registers.rsi = 0;
    thread->gs->ctx->registers.r8  = 0;
    thread->gs->ctx->registers.r9  = 0;
    thread->gs->ctx->registers.r10 = 0;
    thread->gs->ctx->registers.r11 = 0;
    thread->gs->ctx->registers.r12 = 0;
    thread->gs->ctx->registers.r13 = 0;
    thread->gs->ctx->registers.r14 = 0;
    thread->gs->ctx->registers.r15 = 0;
    return thread;
}

void threadContextSwitch(thread_t* thread) {
    uint32_t lo, hi;
    lo = (uint32_t)((uint64_t)thread->gs & 0xFFFFFFFF);
    hi = (uint32_t)((uint64_t)thread->gs >> 32);
    wrmsr(0xC0000102, (uint32_t)lo, (uint32_t)hi);

    pagingSetCR3((void*)thread->gs->ctx->cr3);

    __asm__ volatile("mov rsp, %0;"
                     "mov rcx, %1;"
                     "mov r11, 0x202;"
                     :
                     : "r"((uint64_t)thread->stack),
                       "r"((uint64_t)thread->gs->pc));
    __asm__ volatile("sysretq;");
}