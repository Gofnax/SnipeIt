#include "queue.h"

/* Standard library includes */
#include <stddef.h>

eStatus util_queue_init(Queue* queue, uint32_t capacity)
{
    if(queue == NULL || capacity == 0)
    {
        return eSTATUS_NULL_PARAM;
    }

    queue->buffer = osal_alloc(sizeof(void*) * capacity);
    if(queue->buffer == NULL)
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    queue->capacity = capacity;
    queue->head     = 0;
    queue->tail     = 0;
    queue->size     = 0;

    if(osal_mutex_init(&queue->mutex))
    {
        osal_dealloc(queue->buffer);
        return eSTATUS_SYSTEM_ERROR;
    }

    if(osal_cond_init(&queue->not_empty))
    {
        osal_dealloc(queue->buffer);
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

eStatus util_queue_push(Queue* queue, void* element)
{
    if(queue == NULL || queue->buffer == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    /* Make surte the queue is not full */
    osal_mutex_lock(queue->mutex);
    if(queue->size == queue->capacity)
    {
        osal_mutex_unlock(queue->mutex);
        return eSTATUS_ACTION_FAILED;
    }

    /* Insert an element */
    queue->buffer[queue->tail] = element;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;

    /* Signal the conditional variable (not empty) */
    osal_cond_signal(queue->not_empty);
    osal_mutex_unlock(queue->mutex);

    return eSTATUS_SUCCESSFUL;
}

eStatus util_queue_pop(Queue* queue, void** element)
{
    if(queue == NULL || queue->buffer == NULL || element == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    /* Block the calling thread if the queue is empty */
    osal_mutex_lock(queue->mutex);
    while(queue->size == 0)
    {
        osal_cond_wait(queue->not_empty, queue->mutex);
    }

    /* Extract an element */
    *element = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    osal_mutex_unlock(queue->mutex);

    return eSTATUS_SUCCESSFUL;
}

void util_queue_delete(Queue* queue)
{
    if(queue != NULL)
    {
        osal_mutex_destroy(queue->mutex);
        osal_cond_destroy(queue->not_empty);
        osal_dealloc(queue->buffer);
    }
}