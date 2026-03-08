/* Standard library includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* User library includes */
#include "util/log/log.h"
#include "hal/hal.h"
#include "ddl/ddl.h"

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

    DDLFrame frame; // <-- will not remain
    LOG_INFO("Initializing the DDL layer");
    status = ddl_init(&frame);
    if(status)
    {
        LOG_ERROR("Failed to initialize the DDL layer");
        return 1;
    }

    // THIS WILL CHANGE --------------------------------------------------------
    Event read_event = { .type = eDISTANCE_EVENT_READ };
    ddl_post(eDDL_MODULE_DISTANCE, &read_event);
    ddl_join();
    ddl_delete();
    // THIS WILL CHANGE --------------------------------------------------------

    hal_cleanup();
    log_exit();
    return 0;
}