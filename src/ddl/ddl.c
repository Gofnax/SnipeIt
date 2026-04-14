#include "ddl.h"

/* User library includes */
#include "util/event_bus/event_config.h"
#include "util/event_bus/event_bus.h"
#include "ddl/distance/distance.h"
#include "util/log/log.h"

typedef struct
{
    eStatus (*module_init)(DDLFrame* frame);
    eStatus (*module_post)(Event* event);
    eStatus (*module_end)(void);
    void (*module_join)(void);
    void (*module_delete)(void);
    uint32_t    ao_id;
    Event       subscribe_event;
    const char* module_name;
} DDLModule;

static DDLModule ddl_modules[eDLL_MODULE_COUNT] = {
    [eDDL_MODULE_DISTANCE] = {
        .module_init        = ddl_distance_init,
        .module_post        = ddl_distance_post,
        .module_end         = ddl_distance_end,
        .module_join        = ddl_distance_join,
        .module_delete      = ddl_distance_delete,
        .ao_id              = eAO_DISTANCE,
        .subscribe_event    = { .type = eDISTANCE_EVENT_READ },
        .module_name        = "distance"
    }
};

eStatus ddl_init(DDLFrame* frame)
{
    for(uint32_t module_index = 0; module_index < eDLL_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("Initializing %s module", ddl_modules[module_index].module_name);
        eStatus status = ddl_modules[module_index].module_init(frame);
        if(status)
        {
            return status;
        }
        status = util_event_bus_subscribe(ddl_modules[module_index].ao_id, ddl_post,
                                            module_index, &ddl_modules[module_index].subscribe_event);
        if(status)
        {
            return status;
        }
    }

    return eSTATUS_SUCCESSFUL;
}

eStatus ddl_post(uint32_t module, Event* event)
{
    if(module >= eDLL_MODULE_COUNT)
    {
        return eSTATUS_INVALID_VALUE;
    }

    LOG_DEBUG("Event posted to %s module", ddl_modules[module].module_name);
    return ddl_modules[module].module_post(event);
}

eStatus ddl_end(void)
{
    for(int module_index = 0; module_index < eDLL_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("End event sent to %s module", ddl_modules[module_index].module_name);
        eStatus status = ddl_modules[module_index].module_end();
        if(status)
        {
            return status;
        }
    }

    return eSTATUS_SUCCESSFUL;
}

void ddl_join(void)
{
    for(int module_index = 0; module_index < eDLL_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("Joining %s thread", ddl_modules[module_index].module_name);
        ddl_modules[module_index].module_join();
    }
}

void ddl_delete(void)
{
    for(int module_index = 0; module_index < eDLL_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("Delete %s resources", ddl_modules[module_index].module_name);
        ddl_modules[module_index].module_delete();
    }
}