#ifndef DDL_DISTANCE_CONFIG_H
#define DDL_DISTANCE_CONFIG_H 

/* User library includes */
#include "hal/uart/hal_uart_config.h"

typedef enum eDistanceConfig
{
    eDISTANCE_QUEUE_CAPACITY = 4,
    eDISTANCE_READ_RETRY_MAX = 3,
    eDISTANCE_READ_TIMEOUT_MS = 100,
    eDISTANCE_UART_DEVICE = eUART0_DEVICE
} eDistanceConfig;

#endif