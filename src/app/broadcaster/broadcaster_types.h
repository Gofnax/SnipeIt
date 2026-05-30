#ifndef APP_BROADCASTER_TYPES_H
#define APP_BROADCASTER_TYPES_H

/* User library includes */
#include "util/active_object/active_object.h"
#include "broadcaster_events.h"
#include "ddl/ddl_frame.h"

typedef struct
{
    ActiveObject aobj;
    DDLFrame*    source;          /* live frame, written by DDL modules    */
    DDLFrame*    destination;     /* snapshot frame, written by this AO    */
} BroadcasterObject;

#endif