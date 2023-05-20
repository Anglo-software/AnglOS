#pragma once
#include <basic_includes.h>
#include <stdarg.h>

void vga_putc(uint8_t c);
void vga_print(const char* msg);
void vga_puts(const char* msg);
void vga_printf(const char* format, ...);