#pragma once
#include <basic_includes.h>

void init_sse();

static inline void cli() {
    __asm__ volatile("cli");
}

static inline void sti() {
    __asm__ volatile("sti");
}

static inline void halt() {
    __asm__ volatile("hlt");
}