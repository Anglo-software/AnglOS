#include "tss.h"
#include "../gdt/gdt.h"
#include "boot/cpu/cpu.h"
#include "libc/string.h"
#include "mm/paging/paging.h"
#include "mm/pmm/pmm.h"
#include "threads/spinlock.h"

extern void* page_direct_base;

extern tss_t tss_descriptors[];
tss_t tss_descriptors[NCPU_MAX];

static uint8_t ist_index[NCPU_MAX];
static spinlock lock = SPINLOCK_INITIALIZER;

uint8_t tssAddStack(int num_cpu) {
    if (ist_index[num_cpu] >= 7)
        return 1;

    void* stack = pmalloc(IST_STACK_NUM_PAGES) + (uint64_t)page_direct_base;

    tss_descriptors[num_cpu].ist[ist_index[num_cpu]] =
        (uint64_t)stack + (PAGE_SIZE * IST_STACK_NUM_PAGES);
    ist_index[num_cpu]++;
    return ist_index[num_cpu];
}

void initTSS(int num_cpu) {
    spinLock(&lock);
    ist_index[num_cpu] = 0;
    uint64_t tss_base  = (uint64_t)&tss_descriptors[num_cpu];
    memset((void*)tss_base, 0, sizeof(tss_t));

    tssAddStack(num_cpu);
    tssAddStack(num_cpu);
    tssAddStack(num_cpu);

    tss_descriptors[num_cpu].rsp[0] = tss_descriptors[num_cpu].ist[0];

    tss_descriptors[num_cpu].io_map = sizeof(tss_t);

    cpus[num_cpu].tss               = (void*)tss_base;

    tssReload(gdtInstallTSS(tss_base, num_cpu));
    spinUnlock(&lock);
}

tss_t* tssGet(int num_cpu) { return &tss_descriptors[num_cpu]; }

void* tssGetStack(int num_cpu, uint8_t ist) {
    return (void*)tss_descriptors[num_cpu].ist[ist];
}