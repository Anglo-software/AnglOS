#pragma once
#include <basic_includes.h>

extern int num_cpus;

void initSMP();
void smpStartAP(uint64_t _start_func, uint64_t id);