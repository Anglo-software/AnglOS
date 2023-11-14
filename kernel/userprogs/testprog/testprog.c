#include "libc/string.h"
#include "libc/user/syscall.h"

void _start() {
    char* str = "Entered user space on thread 1\n";
    print(str, strlen(str));
    char tmp[2];
    tmp[1] = 0;
    while (true) {
        char c = getc();
        tmp[0] = c;
        print(tmp, 1);
    }
}