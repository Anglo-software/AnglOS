#include "thread.h"
#include "boot/cpu/cpu.h"
#include "boot/tss/tss.h"
#include "device/io.h"
#include "libc/stdio.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"
#include "threads/spinlock.h"
#include "user/elf.h"
#include <stdatomic.h>

static struct list threadlist;
static atomic_ulong next_thread_id;
static spinlock lock;
extern void* page_direct_base;

extern void threadContextSwitch(thread_t* thread);

static uint64_t getMinRuntime() {
    uint64_t min         = -1;
    struct list* threads = &threadlist;
    thread_t* thread     = NULL;
    spinLock(&lock);
    for (struct list_elem* elem          = list_begin(threads);
         elem != list_end(threads); elem = list_next(elem)) {
        thread = list_entry(elem, thread_t, elem);
        spinLock(&thread->lock);
        if (thread->runtime < min) {
            min = thread->runtime;
        }
        spinUnlock(&thread->lock);
    }
    spinUnlock(&lock);
    return min;
}

void setKernGS(uint64_t gs_base) {
    uint32_t lo, hi;
    lo = (uint32_t)(gs_base & 0xFFFFFFFF);
    hi = (uint32_t)(gs_base >> 32);
    wrmsr(KERN_GS_BASE, (uint32_t)lo, (uint32_t)hi);
}

void setUserGS(uint64_t gs_base) {
    uint32_t lo, hi;
    lo = (uint32_t)(gs_base & 0xFFFFFFFF);
    hi = (uint32_t)(gs_base >> 32);
    wrmsr(USER_GS_BASE, (uint32_t)lo, (uint32_t)hi);
}

uint64_t getKernGS() {
    uint32_t lo, hi;
    rdmsr(KERN_GS_BASE, &lo, &hi);
    return ((uint64_t)lo) | ((uint64_t)hi << 32);
}

uint64_t getUserGS() {
    uint32_t lo, hi;
    rdmsr(USER_GS_BASE, &lo, &hi);
    return ((uint64_t)lo) | ((uint64_t)hi << 32);
}

void initThread() { list_init(&threadlist); }

tid_t threadCreate(void* elf, uint64_t priority, uint64_t parent) {
    uint64_t cpuid         = next_thread_id % ncpu;
    thread_t* thread       = (thread_t*)kmalloc(sizeof(thread_t));
    void* cr3              = pagingCreateUser();
    address_space_t* space = addressSpaceNew((uint64_t)cr3);
    uint64_t oldcr3 = (uint64_t)pagingGetCR3() - (uint64_t)page_direct_base;
    pagingSetCR3((void*)((uint64_t)cr3 - (uint64_t)page_direct_base));
    uint64_t entry = elfLoad(space, elf);
    pagingSetCR3((void*)oldcr3);

    thread->space    = space;
    thread->priority = priority;
    thread->runtime  = getMinRuntime();
    thread->parent   = parent;
    thread->id       = next_thread_id++;
    thread->magic    = THREAD_MAGIC;
    thread->state    = THREAD_BLOCKED;
    thread->stack    = (void*)STACK_START;
    spinLock(&lock);
    list_push_back(&threadlist, &thread->elem);
    spinUnlock(&lock);

    addressSpaceAddEntry(space, ADDRSPACE_TYPE_STACK,
                         ADDRSPACE_FLAG_READABLE | ADDRSPACE_FLAG_USER |
                             ADDRSPACE_FLAG_WRITEABLE,
                         STACK_SIZE, STACK_START - STACK_SIZE);

    thread->gs                     = (gs_base_t*)kmalloc(sizeof(gs_base_t));
    thread->gs->cpu                = (cpu_t*)&cpus[cpuid];
    thread->gs->kernel             = 0;
    thread->gs->pc                 = entry;
    thread->gs->thread             = (void*)thread;
    thread->gs->usr_rsp            = (uint64_t)thread->stack;
    thread->gs->ker_rsp            = (uint64_t)tssGetStack(cpuid, 0);

    thread->gs->ctx                = (ctx_t*)kmalloc(sizeof(ctx_t));
    thread->gs->ctx->cr3           = (uint64_t)cr3 - (uint64_t)page_direct_base;
    thread->gs->ctx->rip           = entry;
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
    return thread->id;
}

void threadSchedule(tid_t tid) {
    struct list* threads = &threadlist;
    thread_t* thread     = NULL;
    spinLock(&lock);
    for (struct list_elem* elem          = list_begin(threads);
         elem != list_end(threads); elem = list_next(elem)) {
        thread = list_entry(elem, thread_t, elem);
        spinLock(&thread->lock);
        if (thread->id == tid) {
            thread->state = THREAD_READY;
            spinUnlock(&thread->lock);
            break;
        }
        spinUnlock(&thread->lock);
    }
    spinUnlock(&lock);
}

void threadBlock(tid_t tid) {
    struct list* threads = &threadlist;
    thread_t* thread     = NULL;
    spinLock(&lock);
    for (struct list_elem* elem          = list_begin(threads);
         elem != list_end(threads); elem = list_next(elem)) {
        thread = list_entry(elem, thread_t, elem);
        spinLock(&thread->lock);
        if (thread->id == tid) {
            thread->state = THREAD_BLOCKED;
            spinUnlock(&thread->lock);
            break;
        }
        spinUnlock(&thread->lock);
    }
    spinUnlock(&lock);
}

void threadKill(tid_t tid) {
    struct list* threads = &threadlist;
    thread_t* thread     = NULL;
    spinLock(&lock);
    for (struct list_elem* elem          = list_begin(threads);
         elem != list_end(threads); elem = list_next(elem)) {
        thread = list_entry(elem, thread_t, elem);
        spinLock(&thread->lock);
        if (thread->id == tid) {
            thread->state = THREAD_DYING;
            spinUnlock(&thread->lock);
            break;
        }
        spinUnlock(&thread->lock);
    }
    spinUnlock(&lock);
}

void threadSelect(tid_t tid) {
    struct list* threads = &threadlist;
    thread_t* thread     = NULL;
    spinLock(&lock);
    for (struct list_elem* elem          = list_begin(threads);
         elem != list_end(threads); elem = list_next(elem)) {
        thread = list_entry(elem, thread_t, elem);
        spinLock(&thread->lock);
        if (thread->id == tid) {
            spinUnlock(&thread->lock);
            break;
        }
        spinUnlock(&thread->lock);
    }
    spinUnlock(&lock);

    cpuCLI();

    setKernGS((uint64_t)thread->gs);
    setUserGS(0);

    threadContextSwitch(thread);
}

tid_t threadNext() {

}

// void threadContextSwitch(thread_t* thread) {
//     cpuCLI();
//     setKernGS((uint64_t)thread->gs);

//     pagingSetCR3((void*)thread->gs->ctx->cr3);

//     __asm__ volatile("mov rsp, %0;"
//                      "mov rcx, %1;"
//                      "mov r11, 0x202;"
//                      :
//                      : "r"((uint64_t)thread->stack),
//                        "r"((uint64_t)thread->gs->pc));
//     cpuSTI();
//     __asm__ volatile("sysretq;");
// }