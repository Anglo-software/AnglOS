#include <libc/string.h>

char *strrchr(const char *s, int c) {
    char* ret = 0;
    do {
        if (*s == (char)c) {
            ret = s;
        }
    } while (*s++);
    return ret;
}