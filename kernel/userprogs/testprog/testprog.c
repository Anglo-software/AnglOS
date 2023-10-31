#include "libc/user/syscall.h"
#include "libc/string.h"

void _start(int input) {
    char* str = "Entered user space\n";
    print(str, strlen(str));
    char tmp[2];
    tmp[1] = 0;
    while (true) {
        char c = getc();
        tmp[0] = c;
        print(tmp, 1);
    }
}