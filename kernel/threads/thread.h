#pragma once
#include <basic_includes.h>
#include "boot/cpu/cpu.h"

typedef struct {
    struct {
        uint64_t rax;
        uint64_t rbx;
        uint64_t rcx;
        uint64_t rdx;
        uint64_t rdi;
        uint64_t rsi;
        uint64_t r8;
        uint64_t r9;
        uint64_t r10;
        uint64_t r11;
        uint64_t r12;
        uint64_t r13;
        uint64_t r14;
        uint64_t r15;
    } registers;

    struct {
        uint64_t rsp;
        uint64_t rbp;
    } stack;

    uint64_t rip;
    uint64_t cr3;
} ctx_t;

typedef struct {
    uint64_t kernel;  // gs:0x00
    uint64_t ker_rsp; // gs:0x08
    uint64_t usr_rsp; // gs:0x10
    uint64_t pc;      // gs:0x18
    cpu_t* cpu;       // gs:0x20
    ctx_t* ctx;       // gs:0x28
} gs_base_t;