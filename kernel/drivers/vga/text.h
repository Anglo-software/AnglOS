#pragma once
#include <basic_includes.h>

void draw_cursor();
void clear_cursor();
void draw_char_at_cursor(uint8_t c, bool transparent);
void draw_char(uint8_t c, uint16_t x, uint16_t y, bool transparent);
void newline();
void cursor_up();
void cursor_down();
void cursor_left();
void cursor_right();
void move_cursor(uint16_t x, uint16_t y);
void advance_cursor_right();
void set_background(uint32_t color);
void set_foreground(uint32_t color);
void clear_char_at_cursor();
void move_screen_up_one_line();
void clear_screen();