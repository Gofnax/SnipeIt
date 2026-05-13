#ifndef DDL_TEMPERATURE_HUMIDITY_TYPES_H
#define DDL_TEMPERATURE_HUMIDITY_TYPES_H

/* Standard library includes */
#include <stdint.h>
#include <stdbool.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "temperature_humidity_events.h"

typedef struct
{
    bool    valid;
} TemperatureHumidityFrame;

#endif