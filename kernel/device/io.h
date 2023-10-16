#pragma once
#include "basic_includes.h"

inline __attribute__((always_inline)) void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("out %1, %0" : : "a"(val), "Nd"(port));
}

inline __attribute__((always_inline)) void outc(uint16_t port, char val)
{
    __asm__ volatile("out %1, %0" : : "a"(val), "Nd"(port));
}

inline __attribute__((always_inline)) uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("in %0, %1" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline __attribute__((always_inline)) void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile("out %1, %0" : : "a"(val), "Nd"(port));
}

inline __attribute__((always_inline)) uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ volatile("in %0, %1" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline __attribute__((always_inline)) void outl(uint16_t port, uint32_t val)
{
    __asm__ volatile("out %1, %0" : : "a"(val), "Nd"(port));
}

inline __attribute__((always_inline)) uint32_t inl(uint16_t port)
{
    uint32_t ret;
    __asm__ volatile("in %0, %1" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline __attribute__((always_inline)) void wrmsr(uint32_t msr, uint32_t lo,
                                                 uint32_t hi)
{
    __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

inline __attribute__((always_inline)) void rdmsr(uint32_t msr, uint32_t* lo,
                                                 uint32_t* hi)
{
    __asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}