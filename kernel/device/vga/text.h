#pragma once
#include <basic_includes.h>

void textDrawCursor();
void textClearCursor();
void textDrawCharAtCursor(uint8_t c, bool transparent);
void textDrawChar(uint8_t c, uint16_t x, uint16_t y, bool transparent);
void textNewline();
void textCursorUp();
void textCursorDown();
void textCursorLeft();
void textCursorRight();
void textMoveCursor(uint16_t x, uint16_t y);
void textAdvanceCursorRight();
void textSetBackground(uint32_t color);
void textSetForeground(uint32_t color);
void textClearCharAtCursor();
void textMoveScreenUpOneLine();
void textClearScreen();
void textPutc(uint8_t c);