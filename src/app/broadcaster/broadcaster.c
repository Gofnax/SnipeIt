#include "broadcaster.h"

/* Standard library includes */
#include <stddef.h>

/* User library includes */
#include "app/broadcaster/broadcaster_types.h"
#include "app/broadcaster/broadcaster_fsm.h"
#include "app/broadcaster/broadcaster_config.h"
#include "util/active_object/active_object.h"

static BroadcasterObject broadcaster_aobj;

eStatus app_broadcaster_configure(DDLFrame* source, DDLFrame* destination)
{
    if(source == NULL || destination == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }
    broadcaster_aobj.source = source;
    broadcaster_aobj.destination = destination;
    return eSTATUS_SUCCESSFUL;
}

eStatus app_broadcaster_init(void)
{
    if(broadcaster_aobj.source == NULL || broadcaster_aobj.destination == NULL)
    {
        return eSTATUS_INVALID_CONFIG;
    }
    return util_active_object_init(&broadcaster_aobj.aobj,
                                   eBROADCASTER_QUEUE_CAPACITY,
                                   broadcaster_init_state);
}

eStatus app_broadcaster_post(Event* event)
{
    return util_active_object_post(&broadcaster_aobj.aobj, event);
}

eStatus app_broadcaster_end(void)
{
    return util_active_object_end(&broadcaster_aobj.aobj);
}

void app_broadcaster_join(void)
{
    util_active_object_join(&broadcaster_aobj.aobj);
}

void app_broadcaster_delete(void)
{
    util_active_object_delete(&broadcaster_aobj.aobj);
}