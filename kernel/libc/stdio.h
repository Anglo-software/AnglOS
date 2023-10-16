#pragma once
#include "debug.h"
#include <basic_includes.h>
#include <stdarg.h>

#include "kernel/stdio.h"

#define STDIN_FILENO  0
#define STDOUT_FILENO 1

/* Standard functions. */
int kprintf(const char*, ...) kprintf_FORMAT(1, 2);
int snkprintf(char*, size_t, const char*, ...) kprintf_FORMAT(3, 4);
int vkprintf(const char*, va_list) kprintf_FORMAT(1, 0);
int vsnkprintf(char*, size_t, const char*, va_list) kprintf_FORMAT(3, 0);
int consolePutchar(int);
int consolePuts(const char*);

/* Nonstandard functions. */
void hex_dump(uintptr_t ofs, const void*, size_t size, bool ascii);
void print_human_readable_size(uint64_t sz);

/* Internal functions. */
void __vkprintf(const char* format, va_list args, void (*output)(char, void*),
                void* aux);
void __kprintf(const char* format, void (*output)(char, void*), void* aux, ...);

/* Try to be helpful. */
#define skprintf  dont_use_skprintf_use_snkprintf
#define vskprintf dont_use_vskprintf_use_vsnkprintf