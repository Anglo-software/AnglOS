#pragma once
#include "basic_includes.h"
#include "boot/cpu/cpu.h"

#define TSS_IST_EXCEPTION   001
#define TSS_IST_ROUTINE     002

#define IST_STACK_NUM_PAGES 4

typedef struct tss {
    uint32_t    rsv0;
    uint64_t    rsp[3];
    uint64_t    rsv1;
    uint64_t    ist[7];
    uint64_t    rsv2;
    uint16_t    rsv3;
    uint16_t    io_map;
} __attribute__((packed)) tss_t;

void    init_tss(int num_cpu);
uint8_t tss_add_stack(int num_cpu);
void    tss_reload(uint16_t selector);
tss_t*  tss_get(int num_cpu);
void*   tss_get_stack(int num_cpu, uint8_t ist);