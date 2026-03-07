/* Standard library includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* User library includes */
#include "util/log/log.h"
#include "ddl/distance/distance.h"
#include "hal/hal.h"

int main(void)
{
    eStatus status = log_init();
    if(status)
    {
        perror("Failed to initialize the log");
        return 1;
    }

    LOG_DEBUG("Initializing the HAL layer");
    status = hal_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize the HAL layer");
    }

    // THIS WILL CHANGE --------------------------------------------------------
    DistanceFrame frame;
    status = ddl_distance_init(&frame);
    if(status)
    {
        perror("Failed to initialize the log");
        return 1;
    }

    Event read_event = { .type = eDISTANCE_EVENT_READ };
    ddl_distance_post(&read_event);
    ddl_distance_join();
    ddl_distance_delete();
    // THIS WILL CHANGE --------------------------------------------------------

    hal_cleanup();
    log_exit();
    return 0;
}