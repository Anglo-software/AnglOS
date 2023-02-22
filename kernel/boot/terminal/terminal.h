#include <basic_includes.h>
#include "../limine.h"

#ifndef TERMINAL_H
#define TERMINAL_H

int init_terminal();
void print(char* str);
void printi(int val, size_t num_digits, uint8_t base);
char* itoa(int value, char* result, int base);

#endif