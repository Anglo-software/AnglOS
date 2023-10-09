#include "syscall.h"
#include "boot/cpu/cpu.h"
#include "device/io.h"
#include "libc/stdio.h"

extern void syscallEntryPoint();

void init_syscall() {
    uint64_t syscall_entry_addr = &syscallEntryPoint;
    uint32_t lo, hi;
    lo = syscall_entry_addr & 0xFFFFFFFF;
    hi = (syscall_entry_addr >> 32) & 0xFFFFFFFF;
    wrmsr(0xC0000082, lo, hi);
    rdmsr(0xC0000080, &lo, &hi);
    lo = lo | 1;
    wrmsr(0xC0000080, lo, hi);
    rdmsr(0xC0000081, &lo, &hi);
    hi = 0x00300008;
    wrmsr(0xC0000081, lo, hi);
}

int syscalltest(int num) {
    printf("Syscall called: %d\n", num);
    return 3;
}

int syscalltest2(int num1, int num2) {
    printf("The sum of %d and %d is %d\n", num1, num2, num1 + num2);
    return 0;
}

int syscalltest3(int n1, int n2, int n3, int n4, int n5, int n6) {
    printf("%d, %d, %d, %d, %d, %d\n", n1, n2, n3, n4, n5, n6);
}