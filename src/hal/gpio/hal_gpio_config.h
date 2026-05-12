#ifndef HAL_GPIO_CONFIG_H
#define HAL_GPIO_CONFIG_H

#include <stdint.h>

typedef enum eGPIODirection
{
    eGPIO_INPUT,
    eGPIO_OUTPUT
} eGPIODirection;

typedef enum eGPIOPull
{
    eGPIO_PULL_NONE,     // No pull-up or pull-down
    eGPIO_PULLUP,
    eGPIO_PULLDOWN
} eGPIOPull;

typedef enum eGPIOEdge
{
    eGPIO_EDGE_NONE,    // used if the device is not configured for interrupt
    eGPIO_EDGE_RISING,
    eGPIO_EDGE_FALLING,
    eGPIO_EDGE_BOTH
} eGPIOEdge;

typedef enum eGPIODeviceNumber
{
    eGPIO0_DEVICE,
    eGPIO1_DEVICE,
    eGPIO_DEVICE_COUNT
} eGPIODeviceNumber;

typedef enum eGPIOConfig
{
    eGPIO0_PIN_CONFIG = 17,
    eGPIO0_DIRECTION_CONFIG = eGPIO_INPUT,
    eGPIO0_PULL_CONFIG = eGPIO_PULL_NONE,
    eGPIO0_EDGE_CONFIG = eGPIO_EDGE_NONE,

    eGPIO1_PIN_CONFIG = 27,
    eGPIO1_DIRECTION_CONFIG = eGPIO_INPUT,
    eGPIO1_PULL_CONFIG = eGPIO_PULL_NONE,
    eGPIO1_EDGE_CONFIG = eGPIO_EDGE_NONE,
} eGPIOConfig;

#endif