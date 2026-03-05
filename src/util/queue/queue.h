#ifndef UTIL_QUEUE_H
#define UTIL_QUEUE_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "osal/osal.h"
#include "status.h"

typedef struct 
{
    void**   buffer;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    uint32_t size;

    void* mutex;
    void* not_empty;
} Queue;

eStatus util_queue_init(Queue* queue, uint32_t capacity);

eStatus util_queue_push(Queue* queue, void* element);

eStatus util_queue_pop(Queue* queue, void** element);

void util_queue_delete(Queue* queue);

#endif