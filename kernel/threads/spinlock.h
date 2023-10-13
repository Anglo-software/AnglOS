#pragma once
#include <basic_includes.h>

#define BUSY 1
typedef unsigned char spinlock;

#define SPINLOCK_INITIALIZER 0

#define INIT_BACKOFF (2000)
#define MAX_BACKOFF (128 * 2000)

#define barrier() __asm__ volatile("": : :"memory")

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() __asm__ volatile("pause\n": : :"memory")

static inline unsigned short xchg_8(void *ptr, unsigned char x)
{
    __asm__ __volatile__("xchgb %0,%1"
                :"=r" (x)
                :"m" (*(volatile unsigned char *)ptr), "0" (x)
                :"memory");

    return x;
}

static inline void spin_init(spinlock* lock) {
    *lock = SPINLOCK_INITIALIZER;
}

static inline void spin_lock(spinlock* lock)
{
    while (1) {
        if (!xchg_8(lock, BUSY)) return;
        volatile unsigned int wait = INIT_BACKOFF;
        while (*lock) {
            for (int i = 0; i < wait; i++) {}
            if (wait <= MAX_BACKOFF) {
                wait *= 2;
            }
            cpu_relax();
        }
    }
}

static inline void spin_unlock(spinlock* lock)
{
    barrier();
    *lock = 0;
}

static inline int spin_trylock(spinlock* lock)
{
    return xchg_8(lock, BUSY);
}