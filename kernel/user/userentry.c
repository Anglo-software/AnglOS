#include "userentry.h"
#include "syscall.h"

void kerneluserentry() {
    print("Entered user space\n", 19);
    char str[2];
    str[1] = 0;
    while (true) {
        char c = getc();
        str[0] = c;
        print(str, 1);
    }
}