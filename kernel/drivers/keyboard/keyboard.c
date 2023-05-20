#include "keyboard.h"
#include "boot/interrupts/isr.h"
#include "boot/interrupts/idt.h"
#include "drivers/8259/pic.h"
#include "drivers/io.h"
#include "drivers/vga/vga_print.h"

void irq_keyboard_handler(registers_t* registers);

static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static uint8_t prev_scan = 0x00;

static unsigned char* scan_code_1 =       "\0""\0""1234567890-=""\x80""\x09""qwertyuiop[]""\x82"
                                          "\x83""asdfghjkl;'`""\x84""\\""zxcvbnm,./""\x84""*""\x85"" ";
static unsigned char* scan_code_1_shift = "\0""\0""!@#$%^&*()_+""\x80""\x09""QWERTYUIOP{}""\x82"
                                          "\x83""ASDFGHJKL:\"~""\x84""\\""ZXCVBNM<>?""\x84""*""\x85"" ";

int init_keyboard() {
    register_interrupt_handler(IRQ1, irq_keyboard_handler);
    pic_unmask_irq(0x01);
    return 0;
}

uint8_t get_keyboard_status() {
    return inb(KEYBOARD_STATUS_REGISTER);
}

uint8_t get_keyboard_byte() {
    while(!(get_keyboard_status() & 0x1));
    return inb(KEYBOARD_DATA_PORT);
}

int send_keyboard_byte(uint8_t data) {
    while (get_keyboard_status() & 0x2);
    outb(KEYBOARD_DATA_PORT, data);
    return 0;
}

int send_keyboard_command(uint8_t command, bool has_ack) {
    if (!(get_keyboard_status() & 0x2)) {
        outb(KEYBOARD_COMMAND_PORT, command);
        if (has_ack) {
            while (!(get_keyboard_status() & 0x1));
            if (get_keyboard_byte() == 0xFA) {
                return 0;
            }
            else if (get_keyboard_byte() == 0xFE) {
                return send_keyboard_command(command, has_ack);
            }
            else {
                return -1;
            }
        }
        return 0;
    }
    else {
        return -1;
    }
}

void send_end_of_transmission() {
    send_keyboard_command(0x80, false);
}

bool is_letter(char c) {
    return (0x61 <= c) && (c <= 0x7A);
}

void irq_keyboard_handler(registers_t* registers) {
    int scan = get_keyboard_byte();

    bool released = (scan & 0b10000000) >> 7;
    
    int scan2 = scan & 0b01111111;
    if (!released) {
        if (scan_code_1[scan2] == LEFT_SHIFT) {
            shift_pressed = true;
        }
        else if (scan_code_1[scan2] == LEFT_CONTROL) {
            ctrl_pressed = true;
        }
        else if (scan_code_1[scan2] == LEFT_ALT) {
            alt_pressed = true;
        }
        else if (scan_code_1[scan2] == BACKSPACE) {
            vga_print("\x08 \x08");
        }
        else if (scan_code_1[scan2] == ENTER) {
            vga_putc('\n');
        }
        else if (prev_scan != 0xE0) {
            if (ctrl_pressed && scan_code_1[scan2] == 'c') {
                vga_print("\033[2J\033[H");
                goto end;
            }
            char str[2];
            if (!shift_pressed) {
                str[0] = scan_code_1[scan2];
            }
            else {
                str[0] = scan_code_1_shift[scan2];
            }
            str[1] = '\0';
            vga_print(str);
        }

        else {
            if (scan == 0x48) {
                // Up
                vga_print("\033[1A");
            }
            else if (scan == 0x4B) {
                // Left
                vga_print("\033[1D");
            }
            else if (scan == 0x4D) {
                // Right
                vga_print("\033[1C");
            }
            else if (scan == 0x50) {
                // Down
                vga_print("\033[1B");
            }
        }
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

    end:

    prev_scan = scan;

    send_end_of_transmission();
    pic_send_eoi(0x01);
}