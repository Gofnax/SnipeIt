#ifndef DDL_GPS_TYPES_H
#define DDL_GPS_TYPES_H

/* Standard library includes */
#include <stdint.h>
#include <stdbool.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "gps_events.h"

typedef struct
{
    bool    valid;
    double  latitude;
    double  longitude;
    float   altitude;
    uint8_t fix_type;          /* 0 none, 2 2D, 3 3D, 4 GNSS+DR, 5 time */
    uint8_t num_satellites;
    float   h_acc; 
} GPSFrame;

typedef struct
{
    ActiveObject    aobj;
    GPSFrame*       frame;
    void*           timer;
    uint32_t        retry;
    uint32_t        system_time;
} GPSObject;

#endif