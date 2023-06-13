#include "keyboard.h"
#include "boot/interrupts/isr.h"
#include "device/apic/apic.h"
#include "device/io.h"
#include "libc/stdio.h"

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
    __asm__ volatile ("cli");
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
            printf("\b");
        }
        else if (scan_code_1[scan2] == ENTER) {
            printf("\n");
        }
        else if (prev_scan != 0xE0) {
            if (ctrl_pressed && scan_code_1[scan2] == 'c') {
                printf("\033[2J\033[H");
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
            printf(str);
        }

        else {
            if (scan == 0x48) {
                // Up
                printf("\033[1A");
            }
            else if (scan == 0x4B) {
                // Left
                printf("\033[1D");
            }
            else if (scan == 0x4D) {
                // Right
                printf("\033[1C");
            }
            else if (scan == 0x50) {
                // Down
                printf("\033[1B");
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

    apic_send_eoi();
    __asm__ volatile ("sti");
}