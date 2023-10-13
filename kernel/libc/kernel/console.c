#include "console.h"
#include "stdarg.h"
#include "libc/stdio.h"
#include "device/vga/text.h"
#include "threads/spinlock.h"

static void vprintf_helper(char, void*);
static void putcharLocked(uint8_t c);

static int64_t write_cnt;

static enum console_mode current_mode = NORMAL_MODE;

static spinlock spin;

void init_console() {
    spin_init(&spin);
}

void console_set_mode(enum console_mode mode) {
    current_mode = mode;
}

void console_print_stats() {
    printf ("Console: %lld characters output\n", write_cnt);
}

static void acquire_console() {
    spin_lock(&spin);
}

static void release_console() {
    spin_unlock(&spin);
}

int vprintf(const char* format, va_list args) {
    int char_cnt = 0;

    __vprintf(format, args, vprintf_helper, &char_cnt);

    return char_cnt;
}

int puts(const char* s) {
    acquire_console();
    while (*s != '\0') {
        putcharLocked(*s++);
    }
    putcharLocked('\n');
    release_console();
    return 0;
}

void putbuf(const char* buffer, size_t n) {
    acquire_console();
    while (n-- > 0) {
        putcharLocked(*buffer++);
    }
    release_console();
}

int putchar(int c) {
    acquire_console();
    putcharLocked(c);
    release_console();
    return c;
}

static void vprintf_helper(char c, void* char_cnt_) {
    acquire_console();
    int *char_cnt = char_cnt_;
    (*char_cnt)++;
    putcharLocked(c);
    release_console();
}

static void putcharLocked(uint8_t c) {
    write_cnt++;
    vga_putc(c);
}