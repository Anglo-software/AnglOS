#include "spinlock.h"
#include <stdatomic.h>
#include "boot/cpu/cpu.h"

void spin_init(spin_t* lock) {
    *lock = false;
}

void spin_lock(spin_t* lock) {
    cli();
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        __builtin_ia32_pause();
    }
}

bool spin_trylock(spin_t* lock) {
    cli();

    if (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        sti();
        return false;
    }
    else {
        return true;
    }
}

void spin_unlock(spin_t* lock) {
    atomic_flag_clear_explicit(lock, memory_order_release);
    sti();
}