#include "syscall.h"

int halt() {
    uint64_t rax;
    save_regs();
    syscall0(0);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

int print(char* str, size_t len) {
    uint64_t rax;
    save_regs();
    syscall2(1, str, len);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

char getc() {
    uint64_t rax;
    save_regs();
    syscall0(2);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}