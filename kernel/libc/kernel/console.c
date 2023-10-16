#include "console.h"
#include "device/vga/text.h"
#include "libc/stdio.h"
#include "stdarg.h"
#include "threads/spinlock.h"

static void vkprintf_helper(char, void*);
static void consolePutcharLocked(uint8_t c);

static int64_t write_cnt;

static enum console_mode current_mode = NORMAL_MODE;

static spinlock spin;

void initConsole() { spinInit(&spin); }

void consoleSetMode(enum console_mode mode) { current_mode = mode; }

void consolePrintStats()
{
    kprintf("Console: %ld characters output\n", write_cnt);
}

static void consoleLock() { spinLock(&spin); }

static void consoleUnlock() { spinUnlock(&spin); }

int vkprintf(const char* format, va_list args)
{
    int char_cnt = 0;

    __vkprintf(format, args, vkprintf_helper, &char_cnt);

    return char_cnt;
}

int consolePuts(const char* s)
{
    consoleLock();
    while (*s != '\0') {
        consolePutcharLocked(*s++);
    }
    consolePutcharLocked('\n');
    consoleUnlock();
    return 0;
}

void consolePutbuf(const char* buffer, size_t n)
{
    consoleLock();
    while (n-- > 0) {
        consolePutcharLocked(*buffer++);
    }
    consoleUnlock();
}

int consolePutchar(int c)
{
    consoleLock();
    consolePutcharLocked(c);
    consoleUnlock();
    return c;
}

static void vkprintf_helper(char c, void* char_cnt_)
{
    consoleLock();
    int* char_cnt = char_cnt_;
    (*char_cnt)++;
    consolePutcharLocked(c);
    consoleUnlock();
}

static void consolePutcharLocked(uint8_t c)
{
    write_cnt++;
    textPutc(c);
}