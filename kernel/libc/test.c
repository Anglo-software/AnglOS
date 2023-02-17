#include "test.h"

unsigned long strlen(const char* const str) {
    unsigned long len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}