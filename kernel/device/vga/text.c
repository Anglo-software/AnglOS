#include "text.h"
#include "graphics.h"
#include "libc/string.h"
#include "fs/kmodule.h"

static uint16_t cursor_x         = 4; // x position of cursor, upper left corner, in pixels
static uint16_t cursor_y         = 4; // y position of cursor, upper left corenr, in pixels
static uint16_t margin_x         = 4; // margin of left and right side for text
static uint16_t margin_y         = 4; // margin of top and bottom side for text
static uint16_t spacing_x        = 2;  // spacing between characters on x axis
static uint16_t spacing_y        = 4;  // spacing between lines
static uint16_t character_size_x = 8;  // Size of a single character in pixels on x axis
static uint16_t character_size_y = 16; // Size of a single character in pixels on y axis
static uint32_t background_color = 0x00000000;
static uint32_t foreground_color = 0xFFFFFFFF;

uint8_t* current_font = NULL;

uint32_t pixels_at_cursor[8*16];

void draw_cursor() {
    uint8_t* fb_base = get_fb_base();
    for (int cy = 0; cy < character_size_y; cy++) {
        uint32_t where = cursor_x * get_bytes_per_pixel() + (cursor_y + cy) * get_pitch();
        memcpy(&pixels_at_cursor[cy * character_size_x], &fb_base[where], get_bytes_per_pixel() * character_size_x);
    }
    draw_char_at_cursor(219, false);
}

void clear_cursor() {
    uint8_t* fb_base = get_fb_base();
    for (int cy = 0; cy < character_size_y; cy++) {
        uint32_t where = cursor_x * get_bytes_per_pixel() + (cursor_y + cy) * get_pitch();
        memcpy(&fb_base[where], &pixels_at_cursor[cy * character_size_x], get_bytes_per_pixel() * character_size_x);
    }
    memset(pixels_at_cursor, 0x00, 8*16*4);
}

void draw_char_at_cursor(uint8_t c, bool transparent) {
    draw_char(c, cursor_x, cursor_y, transparent);
}

void draw_char(uint8_t c, uint16_t x, uint16_t y, bool transparent) {
    if (!current_font) {
        kmodule_t* font = find_by_path("/resources/VGA8.F16");
        current_font = (uint8_t*)(font->address);
    }

    uint8_t* font_char = current_font + (int)c * character_size_y;
    uint8_t mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    for (int cy = 0; cy < character_size_y; cy++) {
        for (int cx = 0; cx < character_size_x; cx++) {
            if (transparent) {
                if (font_char[cy] & mask[cx]) putpixel(x + cx, y + cy, foreground_color);
            }
            else {
                putpixel(x + cx, y + cy, (font_char[cy] & mask[cx]) ? foreground_color : background_color);
            }
        }
    }
}

void newline() {
    clear_cursor();
    if (cursor_y + character_size_y * 2U + spacing_y > get_res_y() - margin_y) {
        move_screen_up_one_line();
        move_cursor(margin_x, cursor_y);
    }
    else {
        move_cursor(margin_x, cursor_y + spacing_y + character_size_y);
    }
}

void cursor_up() {
    if (cursor_y == margin_y) {
        return;
    }
    clear_cursor();
    move_cursor(cursor_x, cursor_y - spacing_y - character_size_y);
}

void cursor_down() {
    if (cursor_y + character_size_y * 2U + spacing_y > get_res_y() - margin_y) {
        return;
    }
    clear_cursor();
    move_cursor(cursor_x, cursor_y + spacing_y + character_size_y);
}

void cursor_left() {
    if (cursor_x == margin_x) {
        return;
    }
    clear_cursor();
    move_cursor(cursor_x - spacing_x - character_size_x, cursor_y);
}

void cursor_right() {
    if (cursor_x + character_size_x * 2U + spacing_x > get_res_x() - margin_x) {
        return;
    }
    clear_cursor();
    move_cursor(cursor_x + spacing_x + character_size_x, cursor_y);
}

void move_cursor(uint16_t x, uint16_t y) {
    cursor_x = x;
    cursor_y = y;
    draw_cursor();
}

void advance_cursor_right() {
    if (cursor_y + character_size_y * 2U + spacing_y > get_res_y() - margin_y &&
        cursor_x + character_size_x * 2U + spacing_x > get_res_x() - margin_x) {
        move_screen_up_one_line();
        move_cursor(margin_x, cursor_y);
    }
    else if (cursor_x + character_size_x * 2U + spacing_x > get_res_x() - margin_x) {
        move_cursor(margin_x, cursor_y + spacing_y + character_size_y);
    }
    else {
        move_cursor(cursor_x + spacing_x + character_size_x, cursor_y);
    }
}

void set_background(uint32_t color) {
    background_color = color;
}

void set_foreground(uint32_t color) {
    foreground_color = color;
}

void clear_char_at_cursor() {
    clear_cursor();
    draw_char_at_cursor(' ', false);
    draw_cursor();
}

void move_screen_up_one_line() {
    uint8_t* fb_base = get_fb_base();

    for (uint64_t i = margin_y + spacing_y + character_size_y; i < get_res_y(); i++) {
        memcpy((void*)fb_base, (void*)(fb_base + (spacing_y + character_size_y) * get_pitch()), get_pitch());
        fb_base += get_pitch();
    }

    fb_base = get_fb_base();
    memset((void*)(fb_base + cursor_y * get_pitch()), 0x00, (character_size_y + margin_y) * get_pitch());
}

void clear_screen() {
    clear_cursor();
    memset((void*)get_fb_base(), 0x00, get_res_y() * get_pitch());
}

void vga_putc(uint8_t c) {
    switch (c) {
        case '\n': newline(); break;
        case '\b': cursor_left(); clear_char_at_cursor(); break;
        case 0x86: cursor_up(); break;
        case 0x87: cursor_down(); break;
        case 0x88: cursor_left(); break;
        case 0x89: cursor_right(); break;
        case 0x90: clear_screen(); move_cursor(4, 4); break;
        default:   clear_cursor(); draw_char_at_cursor(c, false); advance_cursor_right(); break;
    }
}