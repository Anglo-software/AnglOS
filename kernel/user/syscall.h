#pragma once
#include <basic_includes.h>

void initSyscall();

#define save_regs()    __asm__ volatile("push rcx; push r11;")

#define restore_regs() __asm__ volatile("pop r11; pop rcx;")

#define syscall0(rax)                             \
    register uint64_t r_rax __asm__("rax") = rax; \
    __asm__ volatile("syscall;" : : "r"(r_rax));

#define syscall1(rax, rdi)                        \
    register uint64_t r_rax __asm__("rax") = rax; \
    register uint64_t r_rdi __asm__("rdi") = rdi; \
    __asm__ volatile("syscall;" : : "r"(r_rax), "r"(r_rdi));

#define syscall2(rax, rdi, rsi)                   \
    register uint64_t r_rax __asm__("rax") = rax; \
    register uint64_t r_rdi __asm__("rdi") = rdi; \
    register uint64_t r_rsi __asm__("rsi") = rsi; \
    __asm__ volatile("syscall;" : : "r"(r_rax), "r"(r_rdi), "r"(r_rsi));

#define syscall3(rax, rdi, rsi, rdx)              \
    register uint64_t r_rax __asm__("rax") = rax; \
    register uint64_t r_rdi __asm__("rdi") = rdi; \
    register uint64_t r_rsi __asm__("rsi") = rsi; \
    register uint64_t r_rdx __asm__("rdx") = rdx; \
    __asm__ volatile("syscall;"                   \
                     :                            \
                     : "r"(r_rax), "r"(r_rdi), "r"(r_rsi), "r"(r_rdx));

#define syscall4(rax, rdi, rsi, rdx, r10)                              \
    register uint64_t r_rax __asm__("rax") = rax;                      \
    register uint64_t r_rdi __asm__("rdi") = rdi;                      \
    register uint64_t r_rsi __asm__("rsi") = rsi;                      \
    register uint64_t r_rdx __asm__("rdx") = rdx;                      \
    register uint64_t r_r10 __asm__("r10") = r10;                      \
    __asm__ volatile("syscall;"                                        \
                     :                                                 \
                     : "r"(r_rax), "r"(r_rdi), "r"(r_rsi), "r"(r_rdx), \
                       "r"(r_r10));

#define syscall5(rax, rdi, rsi, rdx, r10, r8)                          \
    register uint64_t r_rax __asm__("rax") = rax;                      \
    register uint64_t r_rdi __asm__("rdi") = rdi;                      \
    register uint64_t r_rsi __asm__("rsi") = rsi;                      \
    register uint64_t r_rdx __asm__("rdx") = rdx;                      \
    register uint64_t r_r10 __asm__("r10") = r10;                      \
    register uint64_t r_r8 __asm__("r8")   = r8;                       \
    __asm__ volatile("syscall;"                                        \
                     :                                                 \
                     : "r"(r_rax), "r"(r_rdi), "r"(r_rsi), "r"(r_rdx), \
                       "r"(r_r10), "r"(r_r8));

#define syscall6(rax, rdi, rsi, rdx, r10, r8, r9)                      \
    register uint64_t r_rax __asm__("rax") = rax;                      \
    register uint64_t r_rdi __asm__("rdi") = rdi;                      \
    register uint64_t r_rsi __asm__("rsi") = rsi;                      \
    register uint64_t r_rdx __asm__("rdx") = rdx;                      \
    register uint64_t r_r10 __asm__("r10") = r10;                      \
    register uint64_t r_r8 __asm__("r8")   = r8;                       \
    register uint64_t r_r9 __asm__("r9")   = r9;                       \
    __asm__ volatile("syscall;"                                        \
                     :                                                 \
                     : "r"(r_rax), "r"(r_rdi), "r"(r_rsi), "r"(r_rdx), \
                       "r"(r_r10), "r"(r_r8), "r"(r_r9));

inline int halt() {
    uint64_t rax;
    save_regs();
    syscall0(0);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

inline int print(char* str, size_t len) {
    uint64_t rax;
    save_regs();
    syscall2(1, str, len);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

inline char getc() {
    uint64_t rax;
    save_regs();
    syscall0(2);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}