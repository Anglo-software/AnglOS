#include <basic_includes.h>
#include "limine.h"
#include <libc/string.h>
#include "gdt.h"
 
// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.
 
static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 1024*1024
};
 
static void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

void print(struct limine_terminal *terminal, char* str) {
    terminal_request.response->write(terminal, str, strlen(str));
}

void printi(struct limine_terminal *terminal, int val, size_t num_digits, uint8_t base) {
    char buff[num_digits];
    char* str = itoa(val, buff, base);
    print(terminal, str);
}
 
// The following will be our kernel's entry point.
void _start(void) {
    // Ensure we got a terminal
    if (terminal_request.response == NULL ||
        terminal_request.response->terminal_count < 1) {
        done();
    }

    // Esure we got 1MiB of stack space
    if (stack_request.response == NULL) {
        done();
    }

    init_gdt();
 
    // We should now be able to call the Limine terminal to print out
    // a simple "Hello World" to screen.
    struct limine_terminal *terminal = terminal_request.response->terminals[0];

    for (int i = 0; i < 22*47; i++) {
        if (i / 22 == 23 && i % 22 == 0) {
            print(terminal, "                   Welcome to:\n");
        }
        else if (i / 22 == 25 && i % 22 == 0) {
            print(terminal, "                   Unnamed-OS\n");
        }
        else if (i % 22 == 0 && i != 0) {
            print(terminal, "\n");
        }
        print(terminal, "\033[38;2;255;255;255m");
        print(terminal, "\033[48;2;90;90;90m");
        if (i < 0x10) {
            printi(terminal, 0, 1, 16);
            printi(terminal, 0, 1, 16);
        }
        else if (i < 0x100) {
            printi(terminal, 0, 1, 16);
        }
        printi(terminal, i, 5, 16);
        print(terminal, "\033[0m");
    }
 
    // We're done, just hang...
    done();
}