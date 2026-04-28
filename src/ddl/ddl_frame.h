#ifndef DDL_FRAME_H
#define DDL_FRAME_H

/* User library includes */
#include "ddl/distance/distance_types.h"
#include "ddl/servo/servo_types.h"

// This struct will contain a field for every sensor whose
// data we want to transfer to the Android application
typedef struct
{
    DistanceFrame dist_frame;
} DDLFrame;

#endif