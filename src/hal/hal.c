#include "hal.h"

/* User library includes */
#include "util/log/log.h"
#include "gpio/hal_gpio.h"
#include "uart/hal_uart.h"
#include "i2c/hal_i2c.h"

eStatus hal_init(void)
{
    LOG_DEBUG("Initializing GPIO driver");
    eStatus status = hal_gpio_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize GPIO driver, status = %d", status);
        return status;
    }

    LOG_DEBUG("Initializing UART driver");
    status = hal_uart_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize UART driver, status = %d", status);
        return status;
    }

    LOG_DEBUG("Initializing I2C driver");
    status = hal_i2c_init();
    if(status)
    {
        LOG_ERROR("Failed to initialize I2C driver, status = %d", status);
        return status;
    }

    return status;
}

void hal_cleanup(void)
{
    LOG_DEBUG("Cleaning up GPIO driver");
    hal_gpio_cleanup();
    LOG_DEBUG("Cleaning up UART driver");
    hal_uart_cleanup();
    LOG_DEBUG("Cleaning up I2C driver");
    hal_i2c_cleanup();
}