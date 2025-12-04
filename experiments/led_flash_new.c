#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "../src/hal/gpio/hal_gpio.h"

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) // function that handles SIGINT interrupt (happens when we press `ctrl` + `c` to stop the process)
{
    stop = 1;
}

int main(void)
{
    signal(SIGINT, handle_sigint);
    struct timespec ts = {0, 500000000};

    eHALReturnValue ret_val;

    ret_val = hal_gpio_init();
    if(ret_val != eRETURN_SUCCESS)
    {
        printf("Failed to initialize: %d\n", ret_val);
        return -1;
    }

    ret_val = hal_gpio_write(0, 0);
    if(ret_val != eRETURN_SUCCESS)   // Reset the line
    {
        printf("Failed to write: %d\n", ret_val);
        return -1;
    }

    while(!stop)
    {
        hal_gpio_write(0, 1);
        nanosleep(&ts, NULL);
        hal_gpio_write(0, 0);
        nanosleep(&ts, NULL);
    }

    return 0;
}