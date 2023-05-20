#include <libc/string.h>

char* strerror(int errnum) {
    /*
        That is actually interpreting the standard by the letter, not intent.
        We only know about the "C" locale, no more. That's the only mandatory locale anyway.
    */
    return errnum ? "There was an error, but I didn't crash yet!" : "No error.";
}