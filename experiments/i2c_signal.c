/*
    This is experiment code to see if the src/hal/i2c library works.
    For this we connect the logic analyzer to physical pins 3 (SDA1) and 5 (SCL1) on the Pi 5.
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

#define DEVICE0_ADDRESS 0x40
#define REG_MODE1 0x00  // MODE1 register address on the PCA9685
/*  MODE1 bit description:
    bit 7 - RESTART: 0 - restart disabled; 1 - restart enabled
    bit 6 - EXTCLK: 0 - use internal clock; 1 - use EXTCLK pin clock
    bit 5 - AI: 0 - register auto-increment disabled; 1 - register auto-increment enabled
    bit 4 - SLEEP: 0 - normal mode; 1 - low power mode (oscillator is off)
    bit 3 - SUB1: 0 - device doesn't respond to I2C-bus subaddress 1; 1 - it does respond to it
    bit 2 - SUB2: 0 - device doesn't respond to I2C-bus subaddress 2; 1 - it does respond to it
    bit 1 - SUB3: 0 - device doesn't respond to I2C-bus subaddress 3; 1 - it does respond to it
    bit 0 - ALLCALL: 0 - device doesn't respond to All Call I2C-bus subaddress 1; 1 - it does respond to it
*/

int main(void)
{
    eHALReturnValue ret_val = 0;
    struct timespec ts1 = {1, 0};       // 1 seconds timeframe
    struct timespec ts2 = {0, 500000};  // 500 microseconds timeframe
    printf("Opening I2C devices.\n");
    ret_val = hal_i2c_init();
    ret_val == eRETURN_SUCCESS ? printf("Initialized the I2C device\n") : printf("Failed to initialize I2C device\n");
    if(ret_val != eRETURN_SUCCESS)
    {
        perror("I2C initialization failed");
        return 1;
    }

    hal_i2c_set_address(0, DEVICE0_ADDRESS);

    // Waking up the PCA9685. For this we need to write 
    // '0' to the SLEEP bit
    uint8_t mode1_value = 3;
    ret_val = hal_i2c_read_reg(0, REG_MODE1, 1, &mode1_value, 1);
    ret_val == eRETURN_SUCCESS ? printf("Read MODE1 value\n") : printf("Failed to read MODE1 value\n");
    printf("MODE1 read value: 0x%x\n", mode1_value);
    mode1_value &= ~(1 << 4);   // Setting only bit 4 to '0'
    printf("MODE1 updated value: 0x%x\n", mode1_value);
    ret_val = hal_i2c_write_reg(0, REG_MODE1, 1, &mode1_value, 1);
    ret_val == eRETURN_SUCCESS ? printf("Woke up the device\n") : printf("Failed to wake up device\n");
    nanosleep(&ts2, NULL);  // After changing the SLEEP bit, we need to give the oscillator max of 500microsecs to turn on
    ret_val = hal_i2c_read_reg(0, REG_MODE1, 1, &mode1_value, 1);
    ret_val == eRETURN_SUCCESS ? printf("Read MODE1 value for second time\n") : printf("Failed to read MODE1 value a second time\n");
    printf("MODE1 second read value: 0x%x\n", mode1_value);

    printf("Waiting for ctrl + C\n");
    while(1)
    {
        nanosleep(&ts1, NULL);
    }

    // uint8_t buffer0 = 0x31;
    // while(1)
    // {
    //     printf("Writing 0x%x to I2C device 0\n", buffer0);
    //     ret_val = hal_i2c_write(0, &buffer0, sizeof(buffer0));
    //     printf("ret_val = %d\n", ret_val);
    //     nanosleep(&ts1, NULL);
    // }
}
