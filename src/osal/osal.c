#include "osal.h"

/* Standard library includes */
#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

void* osal_alloc(size_t size)
{
    return malloc(size);
}

void osal_dealloc(void* memory)
{
    free(memory);
}

eStatus osal_mutex_init(void** mutex)
{
    if(mutex == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    *mutex = malloc(sizeof(pthread_mutex_t));
    if(*mutex == NULL)
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    if(pthread_mutex_init((pthread_mutex_t*)(*mutex), NULL))
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
} 

void osal_mutex_lock(void* mutex)
{
    (void)pthread_mutex_lock((pthread_mutex_t*)(mutex));
}

void osal_mutex_unlock(void* mutex)
{
    (void)pthread_mutex_unlock((pthread_mutex_t*)(mutex));
}

void osal_mutex_destroy(void* mutex)
{
    if(mutex != NULL)
    {
        (void)pthread_mutex_destroy((pthread_mutex_t*)(mutex));
        free(mutex);
    }
}

eStatus osal_cond_init(void** cond)
{
    if(cond == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    *cond = malloc(sizeof(pthread_cond_t));
    if(*cond == NULL)
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    if(pthread_cond_init((pthread_cond_t*)(*cond), NULL))
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

void osal_cond_signal(void* cond)
{
    (void)pthread_cond_signal((pthread_cond_t*)(cond));
}

void osal_cond_wait(void* cond, void* mutex)
{
    (void)pthread_cond_wait((pthread_cond_t*)(cond), (pthread_mutex_t*)(mutex));
}

void osal_cond_destroy(void* cond)
{
    if(cond != NULL)
    {
        (void)pthread_cond_destroy((pthread_cond_t*)(cond));
        free(cond);
    }
}

eStatus osal_thread_create(void** thread, EntryFP entry_func, void* arg)
{
    if(thread == NULL || entry_func == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    *thread = malloc(sizeof(pthread_t));
    if(*thread == NULL)
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    if(pthread_create((pthread_t*)(*thread), NULL, entry_func, arg))
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

void osal_thread_join(void* thread)
{
    if(thread != NULL)
    {
        pthread_join(*((pthread_t*)(thread)), NULL);
        free(thread);
    }
}