#pragma once
#include <basic_includes.h>

void initAPICTimer();
void initAPAPICTimer();
uint64_t apicTimerGetResolution();
void timer_start();
void timer_stop();
void apicTimerOneShot(uint64_t time);
void apicTimerPeriodicStart(uint64_t time);
void apicTimerPeriodicStop();
void apicNanosleep(uint64_t nanos);
void apicMillisleep(uint64_t millis);