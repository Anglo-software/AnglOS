#include "libc/string.h"
#include "libc/user/syscall.h"

void _start(int input) {
    char* str = "Entered user space on thread 2\n";
    print(str, strlen(str));
    char tmp[2];
    tmp[1] = 0;
    tmp[0] = '2';
    while (true) {
        print(tmp, 1);
        for (int i = 0; i < 100000000; i++) {
            __asm__ volatile("nop");
        }
        // char c = getc();
        // tmp[0] = c;
        // print(tmp, 1);
    }
}