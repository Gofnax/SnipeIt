#ifndef DDL_TEMPERATURE_HUMIDITY_CONFIG_H
#define DDL_TEMPERATURE_HUMIDITY_CONFIG_H

/* User library includes */
#include "hal/gpio/hal_gpio_config.h"

/* Host start signal: Tbe = 0.8..20 ms. Typical value is 1 ms. */
#define AM2302_START_SIGNAL_HOLD_MS     1

/* Sensor response should begin within Tgo (20..200 µs) after we release
 * the bus, then it pulls low for Trel (75..85 µs) and high for Treh
 * (75..85 µs). We give each phase a generous timeout. */
#define AM2302_RESPONSE_TIMEOUT_US          300
#define AM2302_RESPONSE_PHASE_TIMEOUT_US    150

/* Per-bit timeout: each bit is 50 µs low + (26..75) µs high = ~125 µs max.
 * 200 µs leaves comfortable space for Linux scheduling jitter. */
#define AM2302_BIT_TIMEOUT_US           200

/* Bit-discrimination threshold for the high pulse: spec says "0" is
 * 22..30 µs and "1" is 68..75 µs, so the midpoint at 50 µs is well-
 * separated from both. */
#define AM2302_BIT_THRESHOLD_US         50

/* The sensor's internal sampling rate is 2 seconds.
 * Reads issued at <2 s intervals can return stale data or fail. */
#define AM2302_MIN_READ_INTERVAL_MS     2000

/* Frame structure: 40 bits total = humidity-hi, humidity-lo, temp-hi,
 * temp-lo, checksum (each 8 bits, MSB first). */
#define AM2302_BIT_COUNT                40
#define AM2302_BYTE_COUNT               5

typedef enum eTemperatureHumidityConfig
{
    eTEMPERATURE_HUMIDITY_QUEUE_CAPACITY = 4,
    eTEMPERATURE_HUMIDITY_RETRY_MAX = 3,
    eTEMPERATURE_HUMIDITY_GPIO_DEVICE    = eGPIO0_DEVICE
} eTemperatureHumidityConfig;

#endif