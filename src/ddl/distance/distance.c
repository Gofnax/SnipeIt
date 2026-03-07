#include "distance.h"

/* Standard library includes */
#include <stddef.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "ddl/distance/distance_fsm.h"
#include "ddl/ddl_config.h"
#include "osal/osal.h"

static DistanceObject distance_aobj;

eStatus ddl_distance_init(DistanceFrame* frame)
{
    if(frame == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    distance_aobj.frame = frame;
    distance_aobj.retry = 0;

    return util_active_object_init(
        &distance_aobj.aobj, 
        eDISTANCE_QUEUE_CAPACITY, 
        distance_init_state
    );
}

eStatus ddl_distance_post(Event* event)
{
    return util_active_object_post(&distance_aobj.aobj, event);
}

eStatus ddl_distance_end(void)
{
    return util_active_object_end(&distance_aobj.aobj);
}

void ddl_distance_join(void)
{
    util_active_object_join(&distance_aobj.aobj);
}

void ddl_distance_delete(void)
{
    util_active_object_delete(&distance_aobj.aobj);
    osal_timer_destroy(distance_aobj.timer);
}