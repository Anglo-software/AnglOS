#pragma once
#include <basic_includes.h>
#include "threads/spinlock.h"

#define IQUEUE_BUFSIZE 32

typedef struct {
    spin_t lock;

    uint8_t buf[IQUEUE_BUFSIZE];
    int head;
    int tail;
} iqueue_t;

void init_iqueue(iqueue_t* queue);
void iqueue_lock(iqueue_t* queue);
void iqueue_unlock(iqueue_t* queue);
bool iqueue_empty(iqueue_t* queue);
bool iqueue_full(iqueue_t* queue);
uint8_t iqueue_getc(iqueue_t* queue);
void iqueue_putc(iqueue_t* queue, uint8_t c);