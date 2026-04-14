#ifndef DDL_DISTANCE_EVENTS_H
#define DDL_DISTANCE_EVENTS_H

/* User library includes*/
#include "util/fsm/fsm.h"

typedef enum
{
    eDISTANCE_EVENT_READ = eFSM_EVENT_USER,
    eDISTANCE_EVENT_TIMEOUT,
    eDISTANCE_EVENT_FRAME_RECEIVED
} eDistanceEvent;

#endif