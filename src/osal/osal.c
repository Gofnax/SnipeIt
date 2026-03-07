#include "osal.h"

/* Standard library includes */
#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static void timer_expired_handler(union sigval arg)
{
    TimerArg *timer_arg = arg.sival_ptr;
    timer_arg->handler(timer_arg->arg);
}

static inline void timespec_set_ms(struct timespec *ts, uint64_t ms)
{
    ts->tv_sec  = (__time_t)(ms / 1000);
    ts->tv_nsec = (__time_t)((ms % 1000) * 1000000ULL);
}

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
        free(*mutex);
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
        free(*cond);
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
        free(*thread);
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

void osal_thread_join(void* thread)
{
    if(thread != NULL)
    {
        (void)pthread_join(*((pthread_t*)(thread)), NULL);
        free(thread);
    }
}

void osal_delay_us(uint64_t us)
{
    volatile uint64_t start = monotonic_ns();
    volatile uint64_t duration = us * 1000ull;

    while ((monotonic_ns() - start) < duration) {
        // busy wait
    }
}

void osal_delay_ms(uint64_t ms)
{
    osal_delay_us(ms * 1000ull);
}

eStatus osal_timer_init(void** timer, TimerArg* timer_arg)
{
    if(timer == NULL || timer_arg == NULL || timer_arg->handler == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    *timer = malloc(sizeof(timer_t));
    if(*timer == NULL)
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    struct sigevent sig_event = {
        .sigev_notify            = SIGEV_THREAD,
        .sigev_notify_function   = timer_expired_handler,
        .sigev_notify_attributes = NULL,
        .sigev_value.sival_ptr   = timer_arg
    };

    if(timer_create(CLOCK_MONOTONIC, &sig_event, (timer_t*)(*timer)) == -1)
    {
        free(*timer);
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

eStatus osal_timer_arm(void* timer, uint64_t ms, eTimerType type)
{
    if(timer == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    struct itimerspec its = { 0 };
    timespec_set_ms(&its.it_value, ms);

    if(type == eTIMER_TYPE_REPEAT)
    {
        timespec_set_ms(&its.it_interval, ms);
    }
    else if(type != eTIMER_TYPE_ONCE)
    {
        return eSTATUS_INVALID_VALUE;
    }

    if(timer_settime(*((timer_t*)(timer)), 0, &its, NULL) == -1) 
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

eStatus osal_timer_disarm(void* timer)
{
    // Arming timer to 0 disarms it
    return osal_timer_arm(timer, 0, eTIMER_TYPE_REPEAT);
}

void osal_timer_destroy(void* timer)
{
    if(timer != NULL)
    {
        (void)timer_delete(*((timer_t*)(timer)));
        free(timer);
    }
}