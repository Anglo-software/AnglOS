#include <libc/string.h>

size_t strxfrm(char* dest, const char* src, size_t n) {
    /* This implementation does not know about any locale but "C"... */
    size_t n2 = strlen(src);
    if(n > n2) {
        strcpy(dest, src);
    }
    return n2;
}