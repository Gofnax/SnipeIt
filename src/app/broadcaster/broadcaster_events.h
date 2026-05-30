#ifndef APP_BROADCASTER_EVENTS_H
#define APP_BROADCASTER_EVENTS_H

/* User library includes */
#include "util/fsm/fsm.h"

typedef enum eBroadcasterEvent
{
    eBROADCASTER_EVENT_UPDATE = eFSM_EVENT_USER
} eBroadcasterEvent;

#endif