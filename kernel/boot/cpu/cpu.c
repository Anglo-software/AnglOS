#include "cpu.h"

void initSSE()
{
    __asm__ volatile(".intel_syntax noprefix;"
                     "mov rax, cr0;"
                     "and ax, 0xFFFB;"
                     "or ax, 0x2;"
                     "mov cr0, rax;"
                     "mov rax, cr4;"
                     "or ax, 3 << 9;"
                     "mov cr4, rax;"
                     ".att_syntax prefix;");
}

cpu_t* getCurrentCpu()
{
    cpu_t* cpu;
    __asm__ volatile(".intel_syntax noprefix;"
                     "mov %0, [gs:0x20];"
                     ".att_syntax prefix;"
                     : "=r"(cpu));
    return cpu;
}

bool cpu_can_acquire_spinlock;
cpu_t* bcpu;
cpu_t cpus[NCPU_MAX];
bool cpu_ismp;
uint16_t ncpu;
bool cpu_started_others;