#pragma once
#include <basic_includes.h>

extern int num_cpus;

void init_smp();
void smpStartAP(uint64_t _start_func, uint64_t id);