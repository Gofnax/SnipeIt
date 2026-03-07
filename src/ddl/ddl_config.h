#ifndef DDL_CONFIG_H
#define DDL_CONFIG_H

#include "hal/uart/hal_uart_config.h"
#include "hal/i2c/hal_i2c_config.h"

typedef enum eDistanceConfig
{
    eDISTANCE_QUEUE_CAPACITY = 4,
    eDISTANCE_READ_RETRY_MAX = 3,
    eDISTANCE_READ_TIMEOUT_MS = 100,
    eDISTANCE_UART_DEVICE = eUART0_DEVICE
} eDistanceConfig;

typedef enum eServoMuxConfig
{
    eSERVO_MUX_QUEUE_CAPACITY = 4,
    eSERVO_MUX_I2C_ADDR = 0x08, // TODO: move to implementation later
    eSERVO_MUX_UART_DEVICE = eI2C0_DEVICE
} eServoMuxConfig;

#endif