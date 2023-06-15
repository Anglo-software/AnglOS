#include "keyboard.h"
#include "boot/interrupts/isr.h"
#include "device/apic/apic.h"
#include "device/io.h"
#include "device/input/input.h"
#include "boot/cpu/cpu.h"

void irq_keyboard_handler(registers_t* registers);

static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static uint8_t prev_scan = 0x00;

static unsigned char* scan_code_1 =       (unsigned char*)"\0""\0""1234567890-=""\x80""\x09""qwertyuiop[]""\x82"
                                                          "\x83""asdfghjkl;'`""\x84""\\""zxcvbnm,./""\x84""*""\x85"" ";
static unsigned char* scan_code_1_shift = (unsigned char*)"\0""\0""!@#$%^&*()_+""\x80""\x09""QWERTYUIOP{}""\x82"
                                                          "\x83""ASDFGHJKL:\"~""\x84""\\""ZXCVBNM<>?""\x84""*""\x85"" ";

int init_keyboard() {
    register_interrupt_handler(IRQ1, irq_keyboard_handler);
    ioapic_redirection_t redir = {.vector = IRQ1};
    write_ioapic_redir(0x01, &redir);

    return 0;
}

uint8_t get_keyboard_status() {
    return inb(KEYBOARD_STATUS_REGISTER);
}

uint8_t get_keyboard_byte() {
    return inb(KEYBOARD_DATA_PORT);
}

void send_keyboard_byte(uint8_t data) {
    outb(KEYBOARD_DATA_PORT, data);
}

void send_keyboard_command(uint8_t command) {
    outb(KEYBOARD_COMMAND_PORT, command);
}

bool is_letter(char c) {
    return (0x61 <= c) && (c <= 0x7A);
}

void irq_keyboard_handler(registers_t* registers) {
    int scan = get_keyboard_byte();

    bool released = (scan & 0b10000000) >> 7;
    
    int scan2 = scan & 0b01111111;
    if (!released) {
        char c = 0;
        if (scan_code_1[scan2] == LEFT_SHIFT) {
            shift_pressed = true;
            goto out;
        }
        else if (scan_code_1[scan2] == LEFT_CONTROL) {
            ctrl_pressed = true;
            goto out;
        }
        else if (scan_code_1[scan2] == LEFT_ALT) {
            alt_pressed = true;
            goto out;
        }
        else if (scan_code_1[scan2] == BACKSPACE) {
            c = '\b';
        }
        else if (scan_code_1[scan2] == ENTER) {
            c = '\n';
        }
        else if (prev_scan != 0xE0) {
            if (!ctrl_pressed || scan_code_1[scan2] != 'c') {
                if (!shift_pressed) {
                    c = scan_code_1[scan2];
                }
                else {
                    c = scan_code_1_shift[scan2];
                }
            }
            else {
                c = SCREEN_CLEAR;
            }
        }
        else if (prev_scan == 0xE0) {
            if (scan == 0x48) {
                c = CURSOR_UP;
            }
            else if (scan == 0x4B) {
                c = CURSOR_LEFT;
            }
            else if (scan == 0x4D) {
                c = CURSOR_RIGHT;
            }
            else if (scan == 0x50) {
                c = CURSOR_DOWN;
            }
        }
        input_lock();
        input_putc(c);
        input_unlock();
    }
    else {
        if (scan_code_1[scan2] == LEFT_SHIFT) {
            shift_pressed = false;
        }
        else if (scan_code_1[scan2] == LEFT_CONTROL) {
            ctrl_pressed = false;
        }
        else if (scan_code_1[scan2] == LEFT_ALT) {
            alt_pressed = false;
        }
    }

    out:

    prev_scan = scan;

    apic_send_eoi();
}