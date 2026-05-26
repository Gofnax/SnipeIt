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
    double  latitude;
    double  longitude;
    float   altitude;
    float   h_acc; 
    bool    valid;
    uint8_t fix_type;          /* 0 none, 2 2D, 3 3D, 4 GNSS+DR, 5 time */
    uint8_t num_satellites;
    uint8_t reserved[5];
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