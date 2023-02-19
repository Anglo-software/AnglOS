#include <libc/string.h>

size_t strlen(char* str) {
    size_t len = 0;
    while (str[len] != '\0') { len++; }
    return len;
}