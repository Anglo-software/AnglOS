#include "tss.h"
#include "../gdt/gdt.h"
#include "libc/string.h"
#include "mm/paging/paging.h"
#include "mm/pmm/pmm.h"

extern void* page_direct_base;

extern tss_t tss_descriptors[];
tss_t tss_descriptors[TSS_MAX_CPUS];

static uint8_t ist_index = 0;

uint8_t tss_add_stack(int num_cpu) {
    if (ist_index >= 7)
        return 1;

    void* stack = pmalloc(1) + (uint64_t)page_direct_base;

    tss_descriptors[num_cpu].ist[ist_index] = (uint64_t)stack + PAGE_SIZE;
    ist_index++;
    return ist_index;
}

void init_tss(int num_cpu) {
    uint64_t tss_base = (uint64_t)&tss_descriptors[num_cpu];
    memset((void *)tss_base, 0, sizeof(tss_t));
    
    tss_add_stack(num_cpu);
    tss_add_stack(num_cpu);
    tss_add_stack(num_cpu);

    tss_descriptors[num_cpu].rsp[0] = tss_descriptors[num_cpu].ist[0];

    tss_descriptors[num_cpu].io_map = sizeof(tss_t);

    tss_reload(gdt_install_tss(tss_base));
}
