#include "hal.h"

/* User library includes */
#include "util/log/log.h"
#include "gpio/hal_gpio.h"
#include "uart/hal_uart.h"
#include "i2c/hal_i2c.h"

typedef enum eHALDriver
{
    HAL_DRIVER_GPIO,
    HAL_DRIVER_UART,
    HAL_DRIVER_I2C,
    HAL_DRIVER_COUNT
} eHALDriver;

typedef struct
{
    eStatus (*driver_init)(void);
    void (*driver_cleanup)(void);
    const char* driver_name;
} HALDriver;

static HALDriver hal_drivers[HAL_DRIVER_COUNT] = {
    [HAL_DRIVER_GPIO] = {
        .driver_init    = hal_gpio_init,
        .driver_cleanup = hal_gpio_cleanup,
        .driver_name    = "GPIO"
    },
    [HAL_DRIVER_UART] = {
        .driver_init    = hal_uart_init,
        .driver_cleanup = hal_uart_cleanup,
        .driver_name    = "UART"
    },
    [HAL_DRIVER_I2C] = {
        .driver_init    = hal_i2c_init,
        .driver_cleanup = hal_i2c_cleanup,
        .driver_name    = "I2C"
    }
};

eStatus hal_init(void)
{
    for(int driver_index = 0; driver_index < HAL_DRIVER_COUNT; driver_index++)
    {
        LOG_DEBUG("Initializing %s driver", hal_drivers[driver_index].driver_name);
        eStatus status = hal_drivers[driver_index].driver_init();
        if(status)
        {
            return status;
        }
    }

    return eSTATUS_SUCCESSFUL;
}

void hal_cleanup(void)
{
    for(int driver_index = 0; driver_index < HAL_DRIVER_COUNT; driver_index++)
    {
        LOG_DEBUG("Cleaning up %s driver", hal_drivers[driver_index].driver_name);
        hal_drivers[driver_index].driver_cleanup();
    }
}