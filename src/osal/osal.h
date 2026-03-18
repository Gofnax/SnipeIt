#ifndef OSAL_H
#define OSAL_H

/* Standard library includes */
#include <stdint.h>
#include <stddef.h>

/* User library includes */
#include "status.h"

typedef enum eTimerType
{
    eTIMER_TYPE_ONCE,       // one-time expiration
    eTIMER_TYPE_REPEAT      // repeated intervals
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

/**
 * @brief   Abstract microseconds delay.
 * @param   us Number of microseconds to wait.
 */
void osal_delay_us(uint64_t us);

/**
 * @brief   Abstract miliseconds delay.
 * @param   us Number of miliseconds to wait.
 */
void osal_delay_ms(uint64_t ms);

typedef void (*TimerHandlerFP)(void* arg);

typedef struct
{
    TimerHandlerFP handler;
    void*          arg;
} TimerArg;

/**
 * @brief   Abstract timer initialization.
 * @param   timer A pointer to the address which will hold a timer.
 * @param   timer_arg A pointer to a TimerArg struct.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      timer, timer_arg or timer_arg's handler are NULL
 * @retval  eSTATUS_SYSTEM_ERROR    system couldn't allocate memory or create the timer
 */
eStatus osal_timer_init(void** timer, TimerArg* timer_arg);

/**
 * @brief   Abstract timer arming.
 * @param   timer A pointer to a timer.
 * @param   ms The timer interval in miliseconds.
 * @param   type A value from @ref eTimerType.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      timer is NULL
 * @retval  eSTATUS_INVALID_VALUE   type is not a value from @ref eTimerType
 * @retval  eSTATUS_SYSTEM_ERROR    system couldn't configure timer interval
 */
eStatus osal_timer_arm(void* timer, uint64_t ms, eTimerType type);

/**
 * @brief   Abstract timer disarming.
 * @param   timer A pointer to a timer.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      timer is NULL
 * @retval  eSTATUS_SYSTEM_ERROR    system couldn't configure timer interval
 */
eStatus osal_timer_disarm(void* timer);

/**
 * @brief   Abstract timer destroying.
 * @param   timer A pointer to a timer.
 */
void osal_timer_destroy(void* timer);

#endif