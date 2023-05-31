#pragma once
#include <basic_includes.h>

typedef bool spin_t;

#define SPIN_INIT 0

void spin_lock(spin_t* lock);
void spin_unlock(spin_t* lock);