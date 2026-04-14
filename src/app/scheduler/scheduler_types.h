#ifndef DDL_SCHEDULER_TYPES_H
#define DDL_SCHEDULER_TYPES_H

/* Standard library includes */
#include <stdint.h>
#include <stdbool.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "app/scheduler/scheduler_config.h"
#include "util/event_bus/event_config.h"
#include "scheduler_events.h"

typedef struct
{
    Event*          event;
    eActiveObjectID ao_id;
    bool            active;
    uint8_t         reserved[3];
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