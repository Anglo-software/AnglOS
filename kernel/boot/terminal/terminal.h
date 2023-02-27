#include <basic_includes.h>
#include "../limine.h"

#ifndef TERMINAL_H
#define TERMINAL_H

struct limine_terminal* init_terminal();
void print(char* str);
void printi(int64_t val, size_t num_digits, uint8_t base);
void printui(uint64_t val, size_t num_digits, uint8_t base);
char* itoa(int64_t value, char* result, int base);
char* uitoa(uint64_t value, char* result, int base);

#endif