#include "hal_gpio.h"

/* Standard Libraries */
#include <stdint.h>

/* Linux Specific Libraries */
#include <gpiod.h>  // requires to add the flag '-lgpiod' when compiling

/* User Libraries */
#include "hal_gpio_config.h"

typedef struct
{
    uint8_t pin;    // Associates to GPIO pin on board
    uint8_t mode;   // Specifies pin mode: input or output
    uint8_t pull;   // Specifies the pullup or pulldown activation
    uint8_t edge;   // Specifies edge trigger for device activation
} GPIODevice;

static GPIODevice gpio_devices[eGPIO_DEVICE_COUNT] = 
{
    [eGPIO0_DEVICE] = {
        .pin = 13,
        .mode = GPIO_INPUT,
        .pull = GPIO_NO_PULL,
        .edge = GPIO_EDGE_RISING
    }
};

eHALReturnValue hal_gpio_init(void)
{

}

eHALReturnValue hal_gpio_read_pin(uint32_t device_index, uint8_t pin)
{

}

eHALReturnValue hal_gpio_write_pin(uint32_t device_index, uint8_t pin, bool value)
{

}