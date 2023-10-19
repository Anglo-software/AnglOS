#pragma once
#include "boot/gdt/gdt.h"
#include <basic_includes.h>

#define NCPU_MAX 8

typedef struct {
    uint64_t cpu_id;
    uint64_t lapic_id;
    void* tss;
    void* gdt;
    int started;

    int ncli;
    int intena;
    bool in_ext_int;
    bool yield_on_return;

    uint64_t idle_ticks;
    uint64_t user_ticks;
    uint64_t kern_ticks;
    uint64_t context_switches;
} cpu_t;

extern bool cpu_can_acquire_spinlock;
extern cpu_t* bcpu;
extern cpu_t cpus[NCPU_MAX];
extern bool cpu_ismp;
extern uint16_t ncpu;
extern bool cpu_started_others;

void initSSE();
cpu_t* getCurrentCpu();

static inline void cpuCLI() { __asm__ volatile("cli"); }

static inline void cpuSTI() { __asm__ volatile("sti"); }

static inline void cpuHLT() { __asm__ volatile("hlt"); }
