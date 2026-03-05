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

/**
 * @brief   Initialize a queue.
 * @param   queue A pointer to an uninitialized Queue struct.
 * @param   capacity The number of slots the queue will contain.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      queue is NULL or capacity is 0
 * @retval  eSTATUS_SYSTEM_ERROR    system couldn't allocate memory or initiate the mutex / conditional variable
 */
eStatus util_queue_init(Queue* queue, uint32_t capacity);

/**
 * @brief   Add an element to a queue.
 * @param   queue A pointer to an initialized Queue.
 * @param   element An element to be added to the queue.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      queue is NULL or uninitialized
 * @retval  eSTATUS_ACTION_FAILED   queue is full
 */
eStatus util_queue_push(Queue* queue, void* element);

/**
 * @brief   Remove an element from a queue.
 * @param   queue A pointer to an initialized Queue.
 * @param   element A pointer to an element to be removed from the queue.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      element is NULL or queue is NULL or uninitialized
 */
eStatus util_queue_pop(Queue* queue, void** element);

/**
 * @brief   Delete a queue.
 * @details Frees the memory and destroys the mutex and variable condition.
 * @param   queue A pointer to an initialized Queue.
 */
void util_queue_delete(Queue* queue);

#endif