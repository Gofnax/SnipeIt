#include "app.h"

/* User library includes */
#include "scheduler/scheduler.h"
#include "util/log/log.h"
#include "ddl/ddl.h"

typedef struct
{
    eStatus (*module_init)(void);
    eStatus (*module_post)(Event* event);
    eStatus (*module_end)(void);
    void (*module_join)(void);
    void (*module_delete)(void);
    const char* module_name;
} APPModule;

static APPModule app_modules[eAPP_MODULE_COUNT] = {
    [eAPP_MODULE_SCHEDULER] = {
        .module_init   = app_scheduler_init,
        .module_post   = app_scheduler_post,
        .module_end    = app_scheduler_end,
        .module_join   = app_scheduler_join,
        .module_delete = app_scheduler_delete,
        .module_name   = "scheduler"
    }
};

static DDLFrame ddl_frame;

eStatus app_init(void)
{
    LOG_INFO("Initializing the DDL layer");
    eStatus status = ddl_init(&ddl_frame);
    if(status)
    {
        LOG_ERROR("Failed to initialize the DDL layer");
        return status;
    }

    LOG_INFO("Initializing the APP layer");
    for(int module_index = 0; module_index < eAPP_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("Initializing %s module", app_modules[module_index].module_name);
        status = app_modules[module_index].module_init();
        if(status)
        {
            return status;
        }
    }

    LOG_DEBUG("Registering modules to sequencer");
    static Event distance_read_event = { .type = eDISTANCE_EVENT_READ };
    status = app_scheduler_subscribe(0, ddl_post, eDDL_MODULE_DISTANCE, &distance_read_event); // TODO: hardcoded, refactor later

    return status;
}

eStatus app_post(uint32_t module, Event* event)
{
    if(module >= eAPP_MODULE_COUNT)
    {
        return eSTATUS_INVALID_VALUE;
    }

    LOG_DEBUG("Event posted to %s module", app_modules[module].module_name);
    return app_modules[module].module_post(event);
}

eStatus app_end(void)
{
    eStatus status;

    LOG_INFO("Ending the APP layer");
    for(int module_index = 0; module_index < eAPP_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("End event sent to %s module", app_modules[module_index].module_name);
        status = app_modules[module_index].module_end();
        if(status)
        {
            return status;
        }
    }

    LOG_INFO("Ending the DDL layer");
    status = ddl_end();
    if(status)
    {
        LOG_ERROR("Failed to end DDL layer with status %d", status);
        return status;
    }

    return eSTATUS_SUCCESSFUL;
}

void app_join(void)
{
    LOG_INFO("Joining APP threads");
    for(int module_index = 0; module_index < eAPP_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("Joining %s thread", app_modules[module_index].module_name);
        app_modules[module_index].module_join();
    }

    LOG_INFO("Joining DDL threads");
    ddl_join();
}

void app_delete(void)
{
    LOG_INFO("Deleting APP resources");
    for(int module_index = 0; module_index < eAPP_MODULE_COUNT; module_index++)
    {
        LOG_DEBUG("Delete %s resources", app_modules[module_index].module_name);
        app_modules[module_index].module_delete();
    }

    LOG_INFO("Deleting DDL resources");
    ddl_delete();
}