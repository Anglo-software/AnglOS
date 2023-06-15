#pragma once
#include "basic_includes.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_REGISTER 0x64
#define KEYBOARD_COMMAND_PORT 0x64

#define BACKSPACE 0x80
#define ENTER 0x82
#define LEFT_CONTROL 0x83
#define LEFT_SHIFT 0x84
#define LEFT_ALT 0x85
#define CURSOR_UP 0x86
#define CURSOR_DOWN 0x87
#define CURSOR_LEFT 0x88
#define CURSOR_RIGHT 0x89
#define SCREEN_CLEAR 0x90

int init_keyboard();
void send_keyboard_command(uint8_t);
void send_keyboard_byte(uint8_t);
uint8_t get_keyboard_byte();
uint8_t get_keyboard_status();
void send_end_of_transmission();
bool is_letter(char);