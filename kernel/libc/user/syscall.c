#include "syscall.h"

int halt() {
    uint64_t rax;
    save_regs();
    syscall0(SYS_HALT);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

int print(char* str, size_t len) {
    uint64_t rax;
    save_regs();
    syscall2(SYS_PRINT, str, len);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

char getc() {
    uint64_t rax;
    save_regs();
    syscall0(SYS_GETC);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}

void exit(int code) {
    uint64_t rax;
    save_regs();
    syscall1(SYS_EXIT, code);
    restore_regs();
    __asm__ volatile("mov %0, rax" : "=r"(rax));
    return rax;
}