#include "libc/user/syscall.h"
#include "libc/string.h"

void _start(int input) {
    char* str = "Entered user space on thread 2\n";
    print(str, strlen(str));
    char tmp[2];
    tmp[1] = 0;
    tmp[0] = '2';
    volatile int a = 0;
    while (true) {
        print(tmp, 1);
        for (int i = 0; i < 10000000; i++) {a++;}
        // char c = getc();
        // tmp[0] = c;
        // print(tmp, 1);
    }
}