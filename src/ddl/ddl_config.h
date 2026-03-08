#ifndef DDL_CONFIG_H
#define DDL_CONFIG_H

// MOVE THIS LATER TO SERVO MODULE -----------------------------------
#include "hal/i2c/hal_i2c_config.h"

typedef enum eServoMuxConfig
{
    eSERVO_MUX_QUEUE_CAPACITY = 4,
    eSERVO_MUX_I2C_ADDR = 0x08, // TODO: move to implementation later
    eSERVO_MUX_UART_DEVICE = eI2C0_DEVICE
} eServoMuxConfig;
// MOVE THIS LATER TO SERVO MODULE -----------------------------------

typedef enum eDDLModules
{
    eDDL_MODULE_DISTANCE,
    eDLL_MODULE_COUNT
} eDDLModules;

#endif