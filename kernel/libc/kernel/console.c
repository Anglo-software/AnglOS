#include "console.h"
#include "stdarg.h"
#include "libc/stdio.h"
#include "device/vga/text.h"

static void vprintf_helper(char, void*);
static void putchar_have_lock(uint8_t c);

static int64_t write_cnt;

static enum console_mode current_mode = NORMAL_MODE;

void console_init() {
}

void console_set_mode(enum console_mode mode) {
    current_mode = mode;
}

void console_print_stats() {
    printf ("Console: %lld characters output\n", write_cnt);
}

static void acquire_console() {
}

static void release_console() {
}

static bool console_locked_by_current_thread() {
}

int vprintf(const char* format, va_list args) {
    int char_cnt = 0;

    __vprintf(format, args, vprintf_helper, &char_cnt);

    return char_cnt;
}

int puts(const char* s) {
    while (*s != '\0')
        putchar_have_lock(*s++);
    putchar_have_lock('\n');
    return 0;
}

void putbuf(const char* buffer, size_t n) {
    while (n-- > 0)
        putchar_have_lock(*buffer++);
}

int putchar(int c) {
    putchar_have_lock(c);
    return c;
}

static void vprintf_helper(char c, void* char_cnt_) {
    int *char_cnt = char_cnt_;
    (*char_cnt)++;
    putchar_have_lock(c);
}

static void putchar_have_lock(uint8_t c) {
    write_cnt++;
    vga_putc(c);
}