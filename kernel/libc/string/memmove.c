#include <libc/string.h>

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* pd = dest;
    const unsigned char* ps = src;
    if (ps < pd) {
        for (pd += n, ps += n; n--;) {
            *--pd = *--ps;
        }
    }
    else {
        while(n--) {
            *pd++ = *ps++;
        }
    }
    return dest;
}