/* Standard library includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* User library includes */
#include "util/log/log.h"
#include "ddl/distance/distance.h"

int main(void)
{
    eStatus status = log_init();
    if(status)
    {
        perror("Failed to initialize the log");
        return 1;
    }

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
    log_exit();
    return 0;
}