#ifndef DDL_GPS_CONFIG_H
#define DDL_GPS_CONFIG_H

/* User library includes */
#include "hal/uart/hal_uart_config.h"
 
typedef enum eGpsConfig
{
    eGPS_QUEUE_CAPACITY = 4,
    eDISTANCE_READ_RETRY_MAX = 3,
    eDISTANCE_READ_TIMEOUT_MS = 100,
    eGPS_UART_DEVICE = eUART1_DEVICE
} eGpsConfig;

#endif