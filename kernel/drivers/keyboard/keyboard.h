#include <basic_includes.h>

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_REGISTER 0x64
#define KEYBOARD_COMMAND_PORT 0x64

#define BACKSPACE 0x80
#define ENTER 0x82
#define LEFT_CONTROL 0x83
#define LEFT_SHIFT 0x84
#define LEFT_ALT 0x85

int init_keyboard();
int send_keyboard_command(uint8_t, bool);
int send_keyboard_byte(uint8_t);
uint8_t get_keyboard_byte();
uint8_t get_keyboard_status();
void send_end_of_transmission();
bool is_letter(char);

#endif