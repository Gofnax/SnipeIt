#ifndef DDL_GPS_EVENTS_H
#define DDL_GPS_EVENTS_H

/* User library includes*/
#include "util/fsm/fsm.h"

typedef enum
{
    eGPS_EVENT_READ = eFSM_EVENT_USER,
    eGPS_EVENT_TIMEOUT,
    eGPS_EVENT_FRAME_RECEIVED,
    eGPS_EVENT_CONFIGURED
} eGPSEvent;

#endif