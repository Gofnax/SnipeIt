/* Standard library includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* User library includes */
#include "util/log/log.h"
#include "hal/hal.h"
#include "app/app.h"

#include "app/scheduler/scheduler_types.h"

int main(void)
{
    eStatus status = log_init();
    if(status)
    {
        perror("Failed to initialize the log");
        return 1;
    }

    LOG_INFO("Initializing the HAL layer");
    status = hal_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize the HAL layer");
        return 1;
    }

    status = app_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize the APP layer");
        return 1;
    }

    Event sched_start = { .type = eSCHEDULER_EVENT_START };
    app_post(eAPP_MODULE_SCHEDULER, &sched_start);

    app_join();

    app_delete();
    hal_cleanup();
    log_exit();
    return 0;
}