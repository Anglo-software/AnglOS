#include <libc/string.h>

char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    do {
        if (!n--) {
            return ret;
        }
    } while (*dest++ = *src++);
    while (n--) {
        *dest++ = 0;
    }
    return ret;
}