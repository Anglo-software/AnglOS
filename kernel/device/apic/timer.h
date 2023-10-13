#pragma once
#include <basic_includes.h>

void init_timer();
void init_timer_ap();
uint64_t get_resolution();
void timer_start();
void timer_stop();
void timer_one_shot(uint64_t time);
void timer_periodic_start(uint64_t time);
void timer_periodic_stop();
void nanosleep(uint64_t nanos);
void millisleep(uint64_t millis);