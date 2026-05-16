#ifndef DDL_TEMPERATURE_HUMIDITY_EVENTS_H
#define DDL_TEMPERATURE_HUMIDITY_EVENTS_H

/* User library includes*/
#include "util/fsm/fsm.h"

typedef enum
{
    eTEMPERATURE_HUMIDITY_EVENT_READ = eFSM_EVENT_USER
} eTemperatureHumidityEvent;

#endif