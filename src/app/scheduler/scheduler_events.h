#ifndef APP_SCHEDULER_EVENTS_H
#define APP_SCHEDULER_EVENTS_H

/* User library includes */
#include "util/fsm/fsm.h"

typedef enum eSchedulerEvent
{
    eSCHEDULER_EVENT_START = eFSM_EVENT_USER,
    eSCHEDULER_EVENT_STOP,
    eSCHEDULER_EVENT_TICK
} eSchedulerEvent;

#endif