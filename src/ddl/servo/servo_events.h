#ifndef DDL_SERVO_EVENTS_H
#define DDL_SERVO_EVENTS_H

/* User library includes*/
#include "util/fsm/fsm.h"

typedef enum
{
    eSERVO_EVENT_SCAN = eFSM_EVENT_USER
} eServoEvent;

#endif