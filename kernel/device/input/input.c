#include "input.h"
#include "iqueue.h"

static iqueue_t buffer;

void initInputQueue() { initIQueue(&buffer); }

void inputPutc(uint8_t c) { iqueuePutc(&buffer, c); }

uint8_t inputGetc()
{
    inputLock();
    uint8_t c = iqueueGetc(&buffer);
    inputUnlock();
    return c;
}

bool inputFull() { return iqueueFull(&buffer); }

void inputLock() { iqueueLock(&buffer); }

void inputUnlock() { iqueueUnlock(&buffer); }