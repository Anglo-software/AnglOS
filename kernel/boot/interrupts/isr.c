#include "isr.h"
#include "../terminal/terminal.h"
#include "drivers/8529/pic.h"
#include "drivers/io.h"

void isr_exception_handler(isr_xframe_t* frame);
void isr_exception_handler(isr_xframe_t* frame) {
    print("error: cpu exception ");
    printi(frame->base_frame.vector, 3, 10);
    print(" @ ");
    printi(frame->base_frame.rip, 16, 16);
    print("\n");
    print("error code: ");
    printi((uint64_t)frame->base_frame.error_code, 16, 10);
    __asm__ volatile ("cli; hlt");
}

void isr_keyboard_handler(isr_xframe_t* frame) {
    int scan;
    register int i;
    scan = inb(0x60);

    print("\033[2J\033[H");
    print("Keyboard input detected\n");

    outb(0x61, i|0x80);
    outb(0x61, i);
    pic_send_eoi(0x01);
}