#include "cpu.h"

void init_sse() {
    __asm__ volatile (".intel_syntax noprefix;"
                      "mov rax, cr0;"
                      "and ax, 0xFFFB;"
                      "or ax, 0x2;"
                      "mov cr0, rax;"
                      "mov rax, cr4;"
                      "or ax, 3 << 9;"
                      "mov cr4, rax;"
                      ".att_syntax prefix;");
}