#pragma once
#include <basic_includes.h>

void init_input();
void input_putc(uint8_t c);
uint8_t input_getc();
bool input_full();
void input_lock();
void input_unlock();
