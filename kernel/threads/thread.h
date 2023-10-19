#pragma once
#include "boot/cpu/cpu.h"
#include "libc/kernel/list.h"
#include <basic_includes.h>

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
    void* thread;     // gs:0x30
} gs_base_t;

typedef int64_t tid_t;
enum thread_state {
    THREAD_RUNNING,
    THREAD_READY,
    THREAD_BLOCKED,
    THREAD_DYING
};

#define TID_ERROR    ((tid_t)-1)
#define PRIO_MIN     -20
#define PRIO_DEFAULT 0
#define PRIO_MAX     19
#define THREAD_MAGIC 0xE621CAFEDEADBEEF

typedef struct {
    tid_t id;
    tid_t parent;
    uint64_t priority;
    uint64_t state;
    void* stack;
    struct list_elem elem;
    gs_base_t* gs;
    uint64_t magic;
} thread_t;