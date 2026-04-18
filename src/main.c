/* Standard library includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* User library includes */
#include "util/event_bus/event_list.h"
#include "util/event_bus/event_bus.h"
#include "util/log/log.h"
#include "hal/hal.h"
#include "app/app.h"


int main(void)
{
    eStatus status = log_init();
    if(status)
    {
        perror("Failed to initialize the log");
        return 1;
    }

    status = hal_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize the HAL layer");
        return 1;
    }

    status = util_event_bus_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize the event bus");
        return 1;
    }

    status = app_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize the APP layer");
        return 1;
    }

    Event sched_start = { .type = eSCHEDULER_EVENT_START };
    status = util_event_bus_publish(eAO_SCHEDULER, sched_start.type);
    if(status)
    {
        LOG_ERROR("Failed to publish eSCHEDULER_EVENT_START");
        return 1;
    }

    app_join();

    app_delete();
    util_event_bus_delete();
    hal_cleanup();
    log_exit();
    return 0;
}