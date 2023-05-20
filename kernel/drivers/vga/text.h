#pragma once
#include <basic_includes.h>

void draw_char_at_cursor(uint8_t c, bool transparent);
void draw_char(uint8_t c, uint16_t x, uint16_t y, bool transparent);
void newline();
void move_cursor(uint16_t x, uint16_t y);
void advance_cursor();
void set_background(uint32_t color);
void set_foreground(uint32_t color);
void clear_char_at_cursor();
void move_screen_up_one_line();