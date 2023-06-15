#include "input.h"
#include "iqueue.h"

static iqueue_t buffer;

void init_input() {
    init_iqueue(&buffer);
}

void input_putc(uint8_t c) {
    iqueue_putc(&buffer, c);
}

uint8_t input_getc() {
    input_lock();
    uint8_t c = iqueue_getc(&buffer);
    input_unlock();
    return c;
}

bool input_full() {
    return iqueue_full(&buffer);
}

void input_lock() {
    iqueue_lock(&buffer);
}

void input_unlock() {
    iqueue_unlock(&buffer);
}