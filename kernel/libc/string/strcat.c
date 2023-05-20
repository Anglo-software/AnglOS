#include <libc/string.h>

char* strcat(char* dest, const char* src) {
    char* ret = dest;
    while (*dest) {
        dest++;
    }
    while (*dest++ = *src++) {
        continue;
    }
    return ret;
}