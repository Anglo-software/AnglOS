#pragma once
#include <basic_includes.h>

typedef bool spin_t;

void spin_init(spin_t* lock);
void spin_lock(spin_t* lock);
bool spin_trylock(spin_t* lock);
void spin_unlock(spin_t* lock);