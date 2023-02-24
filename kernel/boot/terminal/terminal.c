#include "terminal.h"
#include "libc/string.h"

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

struct limine_terminal *terminal;

int init_terminal() {
    if (terminal_request.response == NULL) {
        return -1;
    }
    terminal = terminal_request.response->terminals[0];
    return 0;
}

void print(char* str) {
    terminal_request.response->write(terminal, str, strlen(str));
}

void printi(int64_t val, size_t num_digits, uint8_t base) {
    char buff[num_digits];
    char* str = itoa(val, buff, base);
    print(str);
}

void printui(uint64_t val, size_t num_digits, uint8_t base) {
    char buff[num_digits];
    char* str = uitoa(val, buff, base);
    print(str);
}

char* itoa(int64_t value, char* result, int base) {
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int64_t tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

char* uitoa(uint64_t value, char* result, int base) {
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    uint64_t tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}