#ifndef DDL_CONFIG_H
#define DDL_CONFIG_H

typedef enum eDistanceConfig
{
    eDISTANCE_QUEUE_CAPACITY = 4,
    eDISTANCE_READ_RETRY_MAX = 3,
    eDISTANCE_READ_TIMEOUT_MS = 100
} eDistanceConfig;

#endif