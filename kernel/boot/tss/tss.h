#pragma once
#include "basic_includes.h"
#include "boot/cpu/cpu.h"

#define TSS_IST_THREAD      000
#define TSS_IST_EXCEPTION   001
#define TSS_IST_ROUTINE     002

#define IST_STACK_NUM_PAGES 4

typedef struct tss {
    uint32_t rsv0;
    uint64_t rsp[3];
    uint64_t rsv1;
    uint64_t ist[7];
    uint64_t rsv2;
    uint16_t rsv3;
    uint16_t io_map;
} __attribute__((packed)) tss_t;

void initTSS(int num_cpu);
uint8_t tssAddStack(int num_cpu);
void tssReload(uint16_t selector);
tss_t* tssGet(int num_cpu);
void* tssGetStack(int num_cpu, uint8_t ist);