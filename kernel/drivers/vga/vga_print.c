#include "vga_print.h"
#include "text.h"

static const char COVERSION_TABLE[] = "0123456789abcdef";

void vga_putc(uint8_t c) {
    switch (c) {
        case '\n': newline(); break;
        case '\b': cursor_left(); clear_char_at_cursor(); break;
        default:   clear_cursor(); draw_char_at_cursor(c, false); advance_cursor_right(); break;
    }
}

static void vga_putspecial(uint8_t c) {
    clear_cursor(); draw_char_at_cursor(c, false); advance_cursor_right();
}

void vga_print(const char* msg) {
    for (size_t i = 0; msg[i]; i++) {
        if (msg[i] == '\033' && msg[i + 1] == '[') {
            i += 2;
            switch (msg[i]) {
                case 'H': clear_cursor(); move_cursor(4, 4); break;
                default:  break;
            }
            switch (msg[i + 1]) {
                case 'A': cursor_up();    i += 1; break;
                case 'B': cursor_down();  i += 1; break;
                case 'C': cursor_right(); i += 1; break;
                case 'D': cursor_left();  i += 1; break;
                case 'J': clear_screen(); draw_cursor(); i += 1; break;
                default:  break;
            }
        }
        else {
            vga_putc(msg[i]);
        }
    }
}

void vga_puts(const char* msg) {
    vga_print(msg);
}

static void vga_printhex(uint64_t num) {
    int i;
    char buf[17];

    if (!num) {
        vga_print("0");
        return;
    }

    buf[16] = 0;

    for (i = 15; num; i--) {
        buf[i] = COVERSION_TABLE[num % 16];
        num /= 16;
    }

    i++;
    vga_print(&buf[i]);
}

static void vga_printdec(uint64_t num) {
    int i;
    char buf[21] = {0};

    if (!num) {
        vga_putc('0');
        return;
    }

    for (i = 19; num; i--) {
        buf[i] = (num % 10) + 0x30;
        num /= 10;
    }

    i++;
    vga_print(&buf[i]);
}

void vga_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'x': vga_printhex(va_arg(args, uint64_t)); break;
                case 'd': vga_printdec(va_arg(args, uint64_t)); break;
                case 's': vga_print(va_arg(args, char*)); break;
                case '`': format++; vga_putspecial(*format); break;
                default: vga_print("Invalid type specifier\n"); break;
            }
        }
        else {
            vga_putc(*format);
        }
        format++;
    }

    va_end(args);
}