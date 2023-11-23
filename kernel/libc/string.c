#include "string.h"

void* memchr(const void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    while (n--) {
        if (*p != (unsigned char)c) {
            p++;
        }
        else {
            return p;
        }
    }
    return 0;
}

int memcmp(const void* __s1, const void* __s2, size_t __n) {
    const char* _s1 = __s1;
    const char* _s2 = __s2;
    for (size_t i = 0; i < __n; _s1++, i++) {
        if (*_s1 != *_s2)
            return *_s1 - *_s2;
        _s2++;
    }
    return 0;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* dp       = dest;
    const char* sp = src;
    while (n--) {
        *dp++ = *sp++;
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* pd       = dest;
    const unsigned char* ps = src;
    if (ps < pd) {
        for (pd += n, ps += n; n--;) {
            *--pd = *--ps;
        }
    }
    else {
        while (n--) {
            *pd++ = *ps++;
        }
    }
    return dest;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

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

char* strchr(const char* s, int c) {
    while (*s != (char)c) {
        if (!*s++) {
            return 0;
        }
    }
    return (char*)s;
}

int strcmp(const char* __s1, const char* __s2) {
    const signed char* __ss1 = (const signed char*)__s1;
    const signed char* __ss2 = (const signed char*)__s2;
    for (; *__ss1; __ss1++) {
        if (*__ss1 != *__ss2)
            return (signed int)(*__ss1 - *__ss2);
        __ss2++;
    }
    return 0;
}

int strcoll(const char* s1, const char* s2) {
    char t1[1 + strxfrm(0, s1, 0)];
    strxfrm(t1, s1, sizeof(t1));
    char t2[1 + strxfrm(0, s2, 0)];
    strxfrm(t2, s2, sizeof(t2));
    return strcmp(t1, t2);
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while (*dest++ = *src++) {
        continue;
    }
    return ret;
}

size_t strcspn(const char* s1, const char* s2) {
    size_t ret = 0;
    while (*s1) {
        if (strchr(s2, *s1)) {
            return ret;
        }
        else {
            s1++, ret++;
        }
    }
    return ret;
}

char* strerror(int errnum) {
    return errnum ? "There was an error, but I didn't crash yet!" : "No error.";
}

size_t strlen(const char* __s) {
    size_t len = 0;
    for (; !!*__s; __s++, len++)
        ;
    return len;
}

size_t strnlen(const char* string, size_t maxlen) {
    size_t length;

    for (length = 0; string[length] != '\0' && length < maxlen; length++)
        continue;
    return length;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (*dest) {
        dest++;
    }
    while (n--) {
        if (!(*dest++ = *src++)) {
            return ret;
        }
    }
    *dest = 0;
    return ret;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n--) {
        if (*s1++ != *s2++) {
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
        }
    }
    return 0;
}

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

char* strpbrk(const char* s1, const char* s2) {
    while (*s1) {
        if (strchr(s2, *s1++)) {
            return (char*)--s1;
        }
    }
    return 0;
}

char* strrchr(const char* s, int c) {
    char* ret = 0;
    do {
        if (*s == (char)c) {
            ret = s;
        }
    } while (*s++);
    return ret;
}

size_t strspn(const char* s1, const char* s2) {
    size_t ret = 0;
    while (*s1 && strchr(s2, *s1++)) {
        ret++;
    }
    return ret;
}

char* strstr(const char* s1, const char* s2) {
    size_t n = strlen(s2);
    while (*s1) {
        if (!memcmp(s1++, s2, n)) {
            return s1 - 1;
        }
    }
    return 0;
}

char* strtok(char* str, const char* delim) {
    static char* p = 0;
    if (str) {
        p = str;
    }
    else if (!p) {
        return 0;
    }
    str = p + strspn(p, delim);
    p   = str + strcspn(str, delim);
    if (p == str) {
        return p = 0;
    }
    p = *p ? * p = 0, p + 1 : 0;
    return str;
}

size_t strxfrm(char* dest, const char* src, size_t n) {
    /* This implementation does not know about any locale but "C"... */
    size_t n2 = strlen(src);
    if (n > n2) {
        strcpy(dest, src);
    }
    return n2;
}