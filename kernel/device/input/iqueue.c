#include "iqueue.h"
#include "boot/cpu/cpu.h"

static int next(int pos) {
    return (pos + 1) % IQUEUE_BUFSIZE;
}

void init_iqueue(iqueue_t* queue) {
    spin_init(&queue->lock);
    queue->head = 0;
    queue->tail = 0;
}

void iqueue_lock(iqueue_t* queue) {
    spin_lock(&queue->lock);
}

void iqueue_unlock(iqueue_t* queue) {
    spin_unlock(&queue->lock);
}

bool iqueue_empty(iqueue_t* queue) {
    return queue->head == queue->tail;
}

bool iqueue_full(iqueue_t* queue) {
    return next(queue->head) == queue->tail;
}

uint8_t iqueue_getc(iqueue_t* queue) {
    while(iqueue_empty(queue)) {
        iqueue_unlock(queue);
        halt();
        iqueue_lock(queue);
    }

    uint8_t c = queue->buf[queue->tail];
    queue->tail = next(queue->tail);
    return c;
}

void iqueue_putc(iqueue_t* queue, uint8_t c) {
    while(iqueue_full(queue)) {
        iqueue_unlock(queue);
        halt();
        iqueue_lock(queue);
    }

    queue->buf[queue->head] = c;
    queue->head = next(queue->head);
}