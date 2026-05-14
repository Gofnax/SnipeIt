#include "temperature_humidity.h"

/* User library includes */
#include "ddl/temperature_humidity/temperature_humidity_config.h"
#include "ddl/temperature_humidity/temperature_humidity_fsm.h"
#include "util/active_object/active_object.h"
#include "osal/osal.h"

static TemperatureHumidityObject temp_hum_aobj;

eStatus ddl_temperature_humidity_init(DDLFrame* frame)
{
    if(frame == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    temp_hum_aobj.frame = &frame->temp_hum_frame;
    temp_hum_aobj.retry = 0;

    return util_active_object_init(
        &temp_hum_aobj.aobj,
        eTEMPERATURE_HUMIDITY_QUEUE_CAPACITY,
        temperature_humidity_init_state
    );
}

eStatus ddl_temperature_humidity_post(Event* event)
{
    return util_active_object_post(&temp_hum_aobj.aobj, event);
}

eStatus ddl_temperature_humidity_end(void)
{
    return util_active_object_end(&temp_hum_aobj.aobj);
}

void ddl_temperature_humidity_join(void)
{
    util_active_object_join(&temp_hum_aobj.aobj);
}

void ddl_temperature_humidity_delete(void)
{
    util_active_object_delete(&temp_hum_aobj.aobj);
}