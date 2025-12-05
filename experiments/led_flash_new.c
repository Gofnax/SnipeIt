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

/*  To test the GPIO library, we enable devices 0 and 1 configured as INPUT on line 17
    and OUTPUT on line 27, respectively. Then we use a breadboard, connecting line 17
    to the + column, the GND to the - column, attach a resistor and a LED down the column,
    and in its end connect the + to line 27 on the Pi. This way we have message printing
    to the terminal and a LED flashing for physical feedback.
*/
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
    
    int read_val = 0;
    hal_gpio_read(1, &read_val);
    if(read_val == 0)
    {
        printf("Read low signal\n");
    }

    hal_gpio_write(0, 1);   // Write '1' to line 17
    nanosleep(&ts, NULL);
    hal_gpio_read(1, &read_val);

    if(read_val == 1)
    {
        printf("Read works\n");
        hal_gpio_write(0, 0);
        while(!stop)
        {
            hal_gpio_write(0, 1);
            nanosleep(&ts, NULL);
            hal_gpio_write(0, 0);
            nanosleep(&ts, NULL);
        }
    }
    else
    {
        printf("Read failed\n");
    }

    return 0;
}