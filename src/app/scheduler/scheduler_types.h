#ifndef DDL_SCHEDULER_TYPES_H
#define DDL_SCHEDULER_TYPES_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "app/scheduler/scheduler_config.h"
#include "util/fsm/fsm.h"

typedef enum
{
    eSCHEDULER_EVENT_START = eFSM_EVENT_USER,
    eSCHEDULER_EVENT_STOP,
    eSCHEDULER_EVENT_TICK
} eSchedulerEvent;

typedef struct
{
    eStatus (*module_post)(uint32_t module, Event* event);
    Event* event;
    uint32_t module;
    uint32_t padding;
} Subscriber;

typedef struct
{
    ActiveObject aobj;
    void*        timer;
    uint32_t     tick;
    uint32_t     padding;
    Subscriber   subscribers[eSCHEDULER_SUBSCRIBERS_MAX];
} SchedulerObject;

#endif