#include "spinlock.h"
#include <stdatomic.h>

void spin_lock(spin_t* lock) {
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        __builtin_ia32_pause();
    }
}

void spin_unlock(spin_t* lock) {
    atomic_flag_clear_explicit(lock, memory_order_release);
}