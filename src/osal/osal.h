#ifndef OSAL_H
#define OSAL_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "status.h"

typedef enum eTimerType
{
    eTIMER_TYPE_ONCE,
    eTIMER_TYPE_REPEAT
} eTimerType;

/**
 * @brief   Absrtact malloc.
 * @param   size The number of bytes to be allocated.
 */
void* osal_alloc(size_t size);

/**
 * @brief   Absrtact free.
 * @param   memory A pointer to previously allocated memory.
 */
void osal_dealloc(void* memory);


/**
 * @brief   Absrtact mutex initialization.
 * @param   mutex A pointer address at which a mutex will be constructed.
 */
eStatus osal_mutex_init(void** mutex);

/**
 * @brief   Absrtact mutex lock.
 * @param   mutex A pointer to an initialized mutex.
 */
void osal_mutex_lock(void* mutex);

/**
 * @brief   Absrtact mutex unlock.
 * @param   mutex A pointer to an initialized mutex.
 */
void osal_mutex_unlock(void* mutex);

/**
 * @brief   Absrtact mutex destroy.
 * @param   mutex A pointer to an initialized mutex.
 */
void osal_mutex_destroy(void* mutex);


/**
 * @brief   Absrtact conditional variable initialization.
 * @param   cond A pointer address at which a conditional
 *          variable will be constructed.
 */
eStatus osal_cond_init(void** cond);

/**
 * @brief   Absrtact conditional variable signal.
 * @param   cond A pointer to an an initialized conditional variable.
 */
void osal_cond_signal(void* cond);

/**
 * @brief   Absrtact conditional variable wait.
 * @param   cond A pointer to an an initialized conditional variable.
 * @param   mutex A pointer to an initialized mutex.
 */
void osal_cond_wait(void* cond, void* mutex);

/**
 * @brief   Absrtact conditional variable destroy.
 * @param   cond A pointer to an an initialized conditional variable.
 */
void osal_cond_destroy(void* cond);

typedef void* (*EntryFP)(void* arg);

/**
 * @brief   Absrtact thread creation.
 * @param   thread A pointer address at which a thread will be constructed.
 * @param   entry_func A function pointer to the thread's function.
 * @param   arg A pointer to the arg required by entry_func.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      thread or entry_func are NULL 
 * @retval  eSTATUS_SYSTEM_ERROR    system couldn't allocate memory or initiate thread
 */
eStatus osal_thread_create(void** thread, EntryFP entry_func, void* arg);

/**
 * @brief   Absrtact thread join.
 * @param   thread A pointer to an initialized thread.
 */
void osal_thread_join(void* thread);

#endif