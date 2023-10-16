#include "userentry.h"
#include "syscall.h"

void kerneluserentry() {
    int ret = printSyscall(2);
    printSyscall2(ret, 4);
    printSyscall3(1, 2, 3, 4, 5, 6);
    volatile bool a = false;
    while (true) {
        if (a == false) {
            a = true;
        }
        else {
            a = false;
        }
        __asm__ volatile("hlt");
    }

    bool b = a;
}