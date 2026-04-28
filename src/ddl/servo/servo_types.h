#ifndef DDL_SERVO_TYPES_H
#define DDL_SERVO_TYPES_H

/* User library includes */
#include "util/active_object/active_object.h"
#include "servo_events.h"

typedef struct
{
    float   hor_angle;
    float   ver_angle;
} ServoFrame;

typedef struct
{
    ActiveObject    aobj;
    ServoFrame*     frame;
} ServoObject;


#endif