#ifndef DDL_DISTANCE_TYPES_H
#define DDL_DISTANCE_TYPES_H

/* Standard library includes */
#include <stdint.h>
#include <stdbool.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "distance_events.h"

typedef struct
{
    bool     valid;
    uint8_t  reserved[3];
    float    distance;
    uint8_t  status;
    uint8_t  precision;
    uint16_t strength;
} DistanceFrame;

typedef struct
{
    ActiveObject   aobj;
    DistanceFrame* frame;
    void*          timer;
    uint32_t       retry;
    uint32_t       system_time;
} DistanceObject;

#endif