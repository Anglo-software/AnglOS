#pragma once
#include "boot/cpu/cpu.h"
#include "libc/kernel/list.h"
#include "mm/addressspace/addressspace.h"
#include "signal.h"
#include "threads/spinlock.h"
#include <basic_includes.h>

typedef struct {
    struct {
        uint64_t rax; // 0x00
        uint64_t rbx; // 0x08
        uint64_t rcx; // 0x10
        uint64_t rdx; // 0x18
        uint64_t rdi; // 0x20
        uint64_t rsi; // 0x28
        uint64_t r8;  // 0x30
        uint64_t r9;  // 0x38
        uint64_t r10; // 0x40
        uint64_t r11; // 0x48
        uint64_t r12; // 0x50
        uint64_t r13; // 0x58
        uint64_t r14; // 0x60
        uint64_t r15; // 0x68
    } registers;

    uint64_t rip; // 0x70
    uint64_t cr3; // 0x78
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
    THREAD_DYING,
    THREAD_NEW
};

#define TID_ERROR    ((tid_t)-1)
#define PRIO_MIN     -20
#define PRIO_DEFAULT 0
#define PRIO_MAX     19
#define THREAD_TIME  5000 // Default time-slice: 5ms
#define THREAD_MAGIC 0xE621CAFEDEADBEEF
#define STACK_START  0x00007FFFFFFFF000
#define KSTACK_START 0xFFFFFFF000FFF000
#define STACK_SIZE   (2 * PAGE_SIZE)
#define KERN_GS_BASE 0xC0000102
#define USER_GS_BASE 0xC0000101

typedef struct {
    tid_t id;               // 0x00
    tid_t parent;           // 0x08
    uint64_t priority;      // 0x10
    uint64_t state;         // 0x18
    uint64_t runtime;       // 0x20 us
    spinlock lock;          // 0x28
    uint8_t padding[7];     //
    signal_t* signal;       // 0x30
    void* stack;            // 0x38
    struct list_elem elem;  // 0x40
    address_space_t* space; // 0x50
    gs_base_t* gs;          // 0x58
    uint64_t magic;         // 0x60
} __attribute__((packed)) thread_t;

void initThread();
void threadStartTimer(uint64_t microseconds);
void threadStopTimer();

void setKernGS(uint64_t gs_base);
void setUserGS(uint64_t gs_base);
uint64_t getKernGS();
uint64_t getUserGS();
tid_t threadCreate(void* elf, uint64_t priority, uint64_t parent);
void threadSchedule(tid_t tid);
void threadBlock(tid_t tid);
void threadKill(tid_t tid);
void threadSelect(tid_t tid);
tid_t threadNext();