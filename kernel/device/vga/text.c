#include "text.h"
#include "fs/kmodule.h"
#include "graphics.h"
#include "libc/string.h"

static uint16_t cursor_x =
    4; // x position of cursor, upper left corner, in pixels
static uint16_t cursor_y =
    4; // y position of cursor, upper left corenr, in pixels
static uint16_t margin_x  = 4; // margin of left and right side for text
static uint16_t margin_y  = 4; // margin of top and bottom side for text
static uint16_t spacing_x = 2; // spacing between characters on x axis
static uint16_t spacing_y = 4; // spacing between lines
static uint16_t character_size_x =
    8; // Size of a single character in pixels on x axis
static uint16_t character_size_y =
    16; // Size of a single character in pixels on y axis
static uint32_t background_color = 0x00000000;
static uint32_t foreground_color = 0xFFFFFFFF;

uint8_t* current_font            = NULL;

uint32_t pixels_at_cursor[8 * 16];

bool cursor_enabled = true;

void textDrawCursor()
{
    uint8_t* fb_base = graphicsGetFramebufferBase();
    for (int cy = 0; cy < character_size_y; cy++) {
        uint32_t where = cursor_x * graphicsGetBytesPerPixel() +
                         (cursor_y + cy) * GraphicsGetPitch();
        memcpy(&pixels_at_cursor[cy * character_size_x], &fb_base[where],
               graphicsGetBytesPerPixel() * character_size_x);
    }
    textDrawCharAtCursor(219, false);
    cursor_enabled = true;
}

void textClearCursor()
{
    uint8_t* fb_base = graphicsGetFramebufferBase();
    for (int cy = 0; cy < character_size_y; cy++) {
        uint32_t where = cursor_x * graphicsGetBytesPerPixel() +
                         (cursor_y + cy) * GraphicsGetPitch();
        memcpy(&fb_base[where], &pixels_at_cursor[cy * character_size_x],
               graphicsGetBytesPerPixel() * character_size_x);
    }
    memset(pixels_at_cursor, 0x00, 8 * 16 * 4);
    cursor_enabled = false;
}

void textDrawCharAtCursor(uint8_t c, bool transparent)
{
    textDrawChar(c, cursor_x, cursor_y, transparent);
}

void textDrawChar(uint8_t c, uint16_t x, uint16_t y, bool transparent)
{
    if (!current_font) {
        kmodule_t* font = kmoduleFindByPath("/resources/VGA8.F16");
        current_font    = (uint8_t*)(font->address);
    }

    uint8_t* font_char = current_font + (int)c * character_size_y;
    uint8_t mask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    for (int cy = 0; cy < character_size_y; cy++) {
        for (int cx = 0; cx < character_size_x; cx++) {
            if (transparent) {
                if (font_char[cy] & mask[cx])
                    graphicsPutpixel(x + cx, y + cy, foreground_color);
            }
            else {
                graphicsPutpixel(x + cx, y + cy,
                                 (font_char[cy] & mask[cx]) ? foreground_color
                                                            : background_color);
            }
        }
    }
}

void textNewline()
{
    textClearCursor();
    if (cursor_y + character_size_y * 2U + spacing_y >
        graphicsGetYRes() - margin_y) {
        textMoveScreenUpOneLine();
        textMoveCursor(margin_x, cursor_y);
    }
    else {
        textMoveCursor(margin_x, cursor_y + spacing_y + character_size_y);
    }
}

void textCursorUp()
{
    if (cursor_y == margin_y) {
        return;
    }
    textClearCursor();
    textMoveCursor(cursor_x, cursor_y - spacing_y - character_size_y);
}

void textCursorDown()
{
    if (cursor_y + character_size_y * 2U + spacing_y >
        graphicsGetYRes() - margin_y) {
        return;
    }
    textClearCursor();
    textMoveCursor(cursor_x, cursor_y + spacing_y + character_size_y);
}

void textCursorLeft()
{
    if (cursor_x == margin_x) {
        return;
    }
    textClearCursor();
    textMoveCursor(cursor_x - spacing_x - character_size_x, cursor_y);
}

void textCursorRight()
{
    if (cursor_x + character_size_x * 2U + spacing_x >
        graphicsGetXRes() - margin_x) {
        return;
    }
    textClearCursor();
    textMoveCursor(cursor_x + spacing_x + character_size_x, cursor_y);
}

void textMoveCursor(uint16_t x, uint16_t y)
{
    cursor_x = x;
    cursor_y = y;
    textDrawCursor();
}

void textAdvanceCursorRight()
{
    if (cursor_y + character_size_y * 2U + spacing_y >
            graphicsGetYRes() - margin_y &&
        cursor_x + character_size_x * 2U + spacing_x >
            graphicsGetXRes() - margin_x) {
        textMoveScreenUpOneLine();
        textMoveCursor(margin_x, cursor_y);
    }
    else if (cursor_x + character_size_x * 2U + spacing_x >
             graphicsGetXRes() - margin_x) {
        textMoveCursor(margin_x, cursor_y + spacing_y + character_size_y);
    }
    else {
        textMoveCursor(cursor_x + spacing_x + character_size_x, cursor_y);
    }
}

void textSetBackground(uint32_t color) { background_color = color; }

void textSetForeground(uint32_t color) { foreground_color = color; }

void textClearCharAtCursor()
{
    textClearCursor();
    textDrawCharAtCursor(' ', false);
    textDrawCursor();
}

void textMoveScreenUpOneLine()
{
    uint8_t* fb_base = graphicsGetFramebufferBase();

    for (uint64_t i = margin_y + spacing_y + character_size_y;
         i < graphicsGetYRes(); i++) {
        memcpy((void*)fb_base,
               (void*)(fb_base +
                       (spacing_y + character_size_y) * GraphicsGetPitch()),
               GraphicsGetPitch());
        fb_base += GraphicsGetPitch();
    }

    fb_base = graphicsGetFramebufferBase();
    memset((void*)(fb_base + cursor_y * GraphicsGetPitch()), 0x00,
           (character_size_y + margin_y) * GraphicsGetPitch());
}

void textClearScreen()
{
    textClearCursor();
    memset((void*)graphicsGetFramebufferBase(), 0x00,
           graphicsGetYRes() * GraphicsGetPitch());
}

void textPutc(uint8_t c)
{
    switch (c) {
    case '\n': textNewline(); break;
    case '\b':
        textCursorLeft();
        textClearCharAtCursor();
        break;
    case 0x86: textCursorUp(); break;
    case 0x87: textCursorDown(); break;
    case 0x88: textCursorLeft(); break;
    case 0x89: textCursorRight(); break;
    case 0x90:
        textClearScreen();
        textMoveCursor(4, 4);
        break;
    case 0x91: textClearCursor(); break;
    case 0x92: textDrawCursor(); break;
    default:
        textClearCursor();
        textDrawCharAtCursor(c, false);
        textAdvanceCursorRight();
        break;
    }
}