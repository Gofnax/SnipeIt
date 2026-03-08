#include "scheduler.h"

/* Standard library includes */
#include <stddef.h>
#include <string.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "app/scheduler/scheduler_config.h"
#include "app/scheduler/scheduler_types.h"
#include "app/scheduler/scheduler_fsm.h"
#include "osal/osal.h"

static SchedulerObject scheduler_aobj;

eStatus app_scheduler_init(void)
{
    scheduler_aobj.tick = 0;
    
    (void)memset(
        scheduler_aobj.subscribers,
        0,
        eSCHEDULER_SUBSCRIBERS_MAX * sizeof(Subscriber)
    );

    return util_active_object_init(
        &scheduler_aobj.aobj, 
        eSCHEDULER_QUEUE_CAPACITY, 
        scheduler_init_state
    );
}

eStatus app_scheduler_subscribe(uint32_t slot, PostFP post, uint32_t module, Event* event)
{
    if(slot >= eSCHEDULER_SUBSCRIBERS_MAX)
    {  
        return eSTATUS_INVALID_VALUE;
    }
    
    if(post == NULL || event == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    if(scheduler_aobj.subscribers[slot].module_post != NULL)
    {
        return eSTATUS_ACTION_FAILED;
    }

    scheduler_aobj.subscribers[slot].module_post = post;
    scheduler_aobj.subscribers[slot].module      = module;
    scheduler_aobj.subscribers[slot].event       = event;

    return eSTATUS_SUCCESSFUL;
}

eStatus app_scheduler_post(Event* event)
{
    return util_active_object_post(&scheduler_aobj.aobj, event);
}

eStatus app_scheduler_end(void)
{
    return util_active_object_end(&scheduler_aobj.aobj);
}

void app_scheduler_join(void)
{
    util_active_object_join(&scheduler_aobj.aobj);
}

void app_scheduler_delete(void)
{
    util_active_object_delete(&scheduler_aobj.aobj);
    osal_timer_destroy(scheduler_aobj.timer);
}