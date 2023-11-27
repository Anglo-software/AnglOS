#include "thread.h"
#include "boot/cpu/cpu.h"
#include "boot/interrupts/isr.h"
#include "boot/tss/tss.h"
#include "device/apic/apic.h"
#include "device/apic/timer.h"
#include "device/io.h"
#include "libc/stdio.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"
#include "threads/spinlock.h"
#include "user/elf.h"
#include <stdatomic.h>

static struct list threadlist;
static struct list ready[NCPU_MAX];
static atomic_ulong next_thread_id;
static spinlock lock;
static uint64_t timer_ticks;
static uint64_t tick_microseconds;
static thread_t* current_thread = NULL;
extern void* page_direct_base;

extern void threadContextSwitch(thread_t* thread);

static uint64_t getMinRuntime() {
    uint64_t min         = -1UL;
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

static thread_t* getThreadWithMinRuntime() {
    uint64_t min         = -1UL;
    thread_t* minthread  = NULL;
    struct list* threads = &threadlist;
    thread_t* thread     = NULL;
    spinLock(&lock);
    for (struct list_elem* elem          = list_begin(threads);
         elem != list_end(threads); elem = list_next(elem)) {
        thread = list_entry(elem, thread_t, elem);
        spinLock(&thread->lock);
        if (thread->runtime < min) {
            min       = thread->runtime;
            minthread = thread;
        }
        spinUnlock(&thread->lock);
    }
    spinUnlock(&lock);
    return minthread;
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

thread_t* getCurrentThread() { return current_thread; }

static void irqThreadTimerHandler() {
    apicSendEOI();
    timer_ticks++;
    if (current_thread != NULL) {
        current_thread->runtime += tick_microseconds;
    }
    if (getUserGS() != 0UL && getKernGS() == 0UL) {
        threadNext();
    }
}

void initThread() {
    list_init(&threadlist);
    for (int i = 0; i < ncpu; i++) {
        list_init(&ready[i]);
    }
    irqRegisterHandler(IRQ0, (void*)irqThreadTimerHandler);
}

void threadStartTimer(uint64_t microseconds) {
    tick_microseconds = microseconds;
    apicTimerPeriodicStart(microseconds * 1000);
}

void threadStopTimer() { apicTimerPeriodicStop(); }

tid_t threadCreate(void* elf, uint64_t priority, uint64_t parent) {
    uint64_t cpuid         = 0;
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
    if (thread->runtime = -1UL) {
        thread->runtime = 0;
    }
    thread->parent    = parent;
    thread->id        = next_thread_id++;
    thread->magic     = THREAD_MAGIC;
    thread->state     = THREAD_NEW;
    thread->usr_stack = (void*)(STACK_START + 3 * 0x1000000 * thread->id);
    thread->ker_stack = (void*)(KSTACK_START + 3 * 0x1000000 * thread->id);
    thread->irq_stack = (void*)(IRQSTACK_START + 3 * 0x1000000 * thread->id);
    thread->isr_stack = (void*)(EXPSTACK_START + 3 * 0x1000000 * thread->id);
    spinLock(&lock);
    list_push_back(&threadlist, &thread->elem);
    spinUnlock(&lock);

    thread->gs                     = (gs_base_t*)kmalloc(sizeof(gs_base_t));
    thread->gs->cpu                = (cpu_t*)&cpus[cpuid];
    thread->gs->syscall            = 0;
    thread->gs->pc                 = entry;
    thread->gs->thread             = (void*)thread;
    thread->gs->usr_rsp            = (uint64_t)thread->usr_stack;
    thread->gs->ker_rsp            = (uint64_t)thread->ker_stack;
    thread->gs->irq_rsp            = (uint64_t)thread->irq_stack;
    thread->gs->isr_rsp            = (uint64_t)thread->isr_stack;

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

    addressSpaceAddEntry(space, ADDRSPACE_TYPE_STACK,
                         ADDRSPACE_FLAG_READABLE | ADDRSPACE_FLAG_USER |
                             ADDRSPACE_FLAG_WRITEABLE,
                         STACK_SIZE, (uint64_t)thread->usr_stack - STACK_SIZE);

    addressSpaceAddEntry(space, ADDRSPACE_TYPE_STACK,
                         ADDRSPACE_FLAG_READABLE | ADDRSPACE_FLAG_WRITEABLE,
                         STACK_SIZE,
                         (uint64_t)thread->gs->ker_rsp - STACK_SIZE);

    addressSpaceAddEntry(space, ADDRSPACE_TYPE_STACK,
                         ADDRSPACE_FLAG_READABLE | ADDRSPACE_FLAG_WRITEABLE,
                         STACK_SIZE,
                         (uint64_t)thread->gs->irq_rsp - STACK_SIZE);

    addressSpaceAddEntry(space, ADDRSPACE_TYPE_STACK,
                         ADDRSPACE_FLAG_READABLE | ADDRSPACE_FLAG_WRITEABLE,
                         STACK_SIZE,
                         (uint64_t)thread->gs->isr_rsp - STACK_SIZE);

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

    current_thread        = thread;
    current_thread->state = THREAD_RUNNING;

    setUserGS(0);
    setKernGS((uint64_t)thread->gs);
    tss_t* tss                      = tssGet(thread->gs->cpu->cpu_id);
    tss->ist[TSS_IST_ROUTINE - 1]   = (uint64_t)current_thread->irq_stack;
    tss->ist[TSS_IST_EXCEPTION - 1] = (uint64_t)current_thread->isr_stack;

    threadContextSwitch(thread);
}

tid_t threadNext() {
    current_thread->gs->cpu->context_switches++;
    current_thread->state = THREAD_READY;
    thread_t* next_thread = getThreadWithMinRuntime();
    if (next_thread->state == THREAD_NEW) {
        threadSelect(next_thread->id);
    }
    next_thread->state = THREAD_RUNNING;
    current_thread     = next_thread;
    setUserGS((uint64_t)current_thread->gs);
    setKernGS(0UL);
    tss_t* tss                      = tssGet(current_thread->gs->cpu->cpu_id);
    tss->ist[TSS_IST_ROUTINE - 1]   = (uint64_t)current_thread->irq_stack;
    tss->ist[TSS_IST_EXCEPTION - 1] = (uint64_t)current_thread->isr_stack;

    return next_thread->id;
}