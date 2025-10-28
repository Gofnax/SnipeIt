/*
    This is experiment code to see if the src/hal/i2c library works.
    For this we connect the logic analyzer to physical pins 1 (SDA1) and 3 (SCL1) on the Pi 5.
    The purpose of the experiment at this moment (as we don't have the 
    servos that are supposed to connect to the system yet) is to see that we
    can send a signal using the library.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "../src/hal/i2c/hal_i2c.h"
#include "../src/hal/hal_types.h"

#define DEVICE0_ADDRESS 0x53

int main(void)
{
    eHALReturnValue ret_val = 0;
    struct timespec ts = {1, 0};
    printf("Opening I2C devices.\n");
    ret_val = hal_i2c_init();
    printf("%d\n", ret_val);
    if(ret_val != eRETURN_SUCCESS)
    {
        perror("I2C initialization failed");
        return 1;
    }

    hal_i2c_set_address(0, DEVICE0_ADDRESS);

    uint8_t buffer0 = 0x31;
    while(1)
    {
        printf("Writing 0x%x to I2C device 0\n", buffer0);
        ret_val = hal_i2c_write(0, &buffer0, sizeof(buffer0));
        perror("I2C writing failed");
        printf("ret_cal = %d\n", ret_val);
        nanosleep(&ts, NULL);
    }
    // if(ret_val != eRETURN_SUCCESS)
    // {
    //     return 1;
    // }
}