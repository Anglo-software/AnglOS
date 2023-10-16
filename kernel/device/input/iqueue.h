#pragma once
#include "threads/spinlock.h"
#include <basic_includes.h>

#define IQUEUE_BUFSIZE 32

typedef struct {
    spinlock lock;

    uint8_t buf[IQUEUE_BUFSIZE];
    int head;
    int tail;
} iqueue_t;

void initIQueue(iqueue_t* queue);
void iqueueLock(iqueue_t* queue);
void iqueueUnlock(iqueue_t* queue);
bool iqueueEmpty(iqueue_t* queue);
bool iqueueFull(iqueue_t* queue);
uint8_t iqueueGetc(iqueue_t* queue);
void iqueuePutc(iqueue_t* queue, uint8_t c);