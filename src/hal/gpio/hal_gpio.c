#include "hal_gpio.h"

/* Standard Libraries */
#include <stdint.h>

/* Linux Specific Libraries */
#include <gpiod.h>  // requires to add the flag '-lgpiod' when compiling

/* User Libraries */
#include "hal_gpio_config.h"

#define GPIO_CHIP_PATH "/dev/gpiochip0"

static struct gpiod_chip* gpio_chip = NULL;

typedef struct
{
    uint8_t             pin;        // Associates to GPIO pin on board (software pin, not physical pin)
    struct gpiod_line*  line;       // Handle to the specific line
    uint8_t             direction;  // Specifies signal direction: input or output
    uint8_t             pull;       // Specifies the pullup or pulldown activation
    uint8_t             edge;       // Specifies edge trigger for device activation
} GPIODevice;

static GPIODevice gpio_devices[eGPIO_DEVICE_COUNT] = 
{
    [eGPIO0_DEVICE] = {
        .pin = eGPIO0_PIN_CONFIG,
        .direction = eGPIO0_DIRECTION_CONFIG,
        .pull = eGPIO0_PULL_CONFIG,
        .edge = eGPIO0_EDGE_CONFIG
    },

    [eGPIO1_DEVICE] = {
        .pin = eGPIO1_PIN_CONFIG,
        .direction = eGPIO1_DIRECTION_CONFIG,
        .pull = eGPIO1_PULL_CONFIG,
        .edge = eGPIO1_EDGE_CONFIG
    }
};

eHALReturnValue hal_gpio_init(void)
{
    if(gpio_chip = gpiod_chip_open(GPIO_CHIP_PATH) == NULL)
    {
        return eRETURN_DEVICE_ERROR;
    }

    for(int i = 0; i < eGPIO_DEVICE_COUNT; ++i)
    {
        gpio_devices[i].line = gpiod_chip_get_line(gpio_chip, gpio_devices[i].pin);

        struct gpiod_line_request_config config = {
            .consumer = "SnipeItGPIO",
            .request_type = -1,
            .flags = -1
        };

        switch(gpio_devices[i].edge)
        {
        case eGPIO_EDGE_NONE:   // If the device is not configured to be used in interrput mode
            if(gpio_devices[i].direction == eGPIO_INPUT)
            {
                config.request_type = GPIOD_LINE_REQUEST_DIRECTION_INPUT;
            }
            else
            {
                config.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
            }
            break;
        case eGPIO_EDGE_RISING:
            config.request_type = GPIOD_LINE_REQUEST_EVENT_RISING_EDGE;
            break;
        case eGPIO_EDGE_FALLING:
            config.request_type = GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE;
            break;
        case eGPIO_EDGE_BOTH:
            config.request_type = GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES;
            break;
        default:
            return eRETURN_INVALID_PARAMETER;
        }

        switch(gpio_devices[i].pull)
        {
        case eGPIO_PULL_NONE:
            config.flags = GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE;
            break;
        case eGPIO_PULLUP:
            config.flags = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP;
            break;
        case eGPIO_PULLDOWN:
            config.flags = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN;
            break;
        default:
            return eRETURN_INVALID_PARAMETER;
        }

        if(gpiod_line_request(gpio_devices[i].line, &config, 0) < 0)
        {
            return eRETURN_DEVICE_ERROR;
        }
    }

    return eRETURN_SUCCESS;
}

eHALReturnValue hal_gpio_read(uint32_t device_index, bool* buffer)
{

}

eHALReturnValue hal_gpio_write(uint32_t device_index, bool value)
{

}

// TO-DO implementation
eHALReturnValue hal_gpio_set_direction(void)
{

}