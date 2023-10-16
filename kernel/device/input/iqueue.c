#include "iqueue.h"
#include "boot/cpu/cpu.h"

static int next(int pos) { return (pos + 1) % IQUEUE_BUFSIZE; }

void initIQueue(iqueue_t* queue) {
    spinInit(&queue->lock);
    queue->head = 0;
    queue->tail = 0;
}

void iqueueLock(iqueue_t* queue) { spinLock(&queue->lock); }

void iqueueUnlock(iqueue_t* queue) { spinUnlock(&queue->lock); }

bool iqueueEmpty(iqueue_t* queue) { return queue->head == queue->tail; }

bool iqueueFull(iqueue_t* queue) { return next(queue->head) == queue->tail; }

uint8_t iqueueGetc(iqueue_t* queue) {
    while (iqueueEmpty(queue)) {
        iqueueUnlock(queue);
        halt();
        iqueueLock(queue);
    }

    uint8_t c   = queue->buf[queue->tail];
    queue->tail = next(queue->tail);
    return c;
}

void iqueuePutc(iqueue_t* queue, uint8_t c) {
    while (iqueueFull(queue)) {
        iqueueUnlock(queue);
        halt();
        iqueueLock(queue);
    }

    queue->buf[queue->head] = c;
    queue->head             = next(queue->head);
}