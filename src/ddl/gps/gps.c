#include "gps.h"

/* User library includes */
#include "util/active_object/active_object.h"
#include "ddl/gps/gps_config.h"
#include "ddl/gps/gps_fsm.h"
#include "osal/osal.h"

static GPSObject gps_aobj;

eStatus ddl_gps_init(DDLFrame* frame)
{
    if(frame == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    gps_aobj.frame = &frame->gps_frame;
    gps_aobj.retry = 0;

    return util_active_object_init(
        &gps_aobj.aobj,
        eGPS_QUEUE_CAPACITY,
        gps_init_state
    );
}

eStatus ddl_gps_post(Event* event)
{
    return util_active_object_post(&gps_aobj.aobj, event);
}

eStatus ddl_gps_end(void)
{
    return util_active_object_end(&gps_aobj.aobj);
}

void ddl_gps_join(void)
{
    util_active_object_join(&gps_aobj);
}

void ddl_gps_delete(void)
{
    util_active_object_delete(&gps_aobj.aobj);
    osal_timer_destroy(gps_aobj.timer);
}