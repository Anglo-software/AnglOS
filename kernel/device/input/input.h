#pragma once
#include <basic_includes.h>

void initInputQueue();
void inputPutc(uint8_t c);
uint8_t inputGetc();
bool inputFull();
void inputLock();
void inputUnlock();
