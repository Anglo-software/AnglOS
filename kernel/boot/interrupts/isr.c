#include "isr.h"
#include "../terminal/terminal.h"

void isr_common_handler(isr_xframe_t* frame) {
    print("error: cpu exception ");
    printi(frame->base_frame.vector, 3, 10);
    print(" @ ");
    printi(frame->base_frame.rip, 16, 16);
    print("\n");
    print("error code: ");
    printi((uint64_t)frame->base_frame.error_code, 16, 10);
    __asm__ volatile ("cli; hlt");
}