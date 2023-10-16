#pragma once
#include "basic_includes.h"
#include "device/apic/apic.h"

#define KEYBOARD_DATA_PORT       0x60
#define KEYBOARD_STATUS_REGISTER 0x64
#define KEYBOARD_COMMAND_PORT    0x64

#define BACKSPACE                0x80
#define ENTER                    0x82
#define LEFT_CONTROL             0x83
#define LEFT_SHIFT               0x84
#define LEFT_ALT                 0x85
#define textCursorUp             0x86
#define textCursorDown           0x87
#define textCursorLeft           0x88
#define textCursorRight          0x89
#define SCREEN_CLEAR             0x90
#define CURSOR_EN                0x91
#define CURSOR_DS                0x92

#define KEYBOARD_IRQ             33

int initKeyboard();
void keyboardSetRedir(ioapic_redirection_t* redir);
void keyboardSendCommand(uint8_t);
void keyboardSendByte(uint8_t);
uint8_t keyboardGetByte();
uint8_t keyboardGetStatus();
void send_end_of_transmission();
bool keyboardIsLetter(char);