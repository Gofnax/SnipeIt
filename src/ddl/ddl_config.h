#ifndef DDL_CONFIG_H
#define DDL_CONFIG_H

#include "hal/uart/hal_uart_config.h"

typedef enum eDistanceConfig
{
    eDISTANCE_QUEUE_CAPACITY = 4,
    eDISTANCE_READ_RETRY_MAX = 3,
    eDISTANCE_READ_TIMEOUT_MS = 100,
    eDISTANCE_UART_DEVICE = eUART0_DEVICE
} eDistanceConfig;

#endif