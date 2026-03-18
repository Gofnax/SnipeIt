#ifndef HAL_GPIO_H
#define HAL_GPIO_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "status.h"

/**
 * @brief   Sets the GPIO devices configuration.
 * @details Opens the GPIO chip and configures all GPIO devices according to
 *          configuration specified in 'hal_gpio_config.h'. Requests and
 *          configures GPIO lines for each device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE failure to define pull or edge configuration
 * @retval  eSTATUS_DEVICE_ERROR  failure to open chip or configure lines
 */
eStatus hal_gpio_init(void);

/**
 * @brief   Read the value of a GPIO pin.
 * @details Reads the current value (high/low) from the GPIO pin associated
 *          with the specified device.
 * @param   device_index A value from @ref eGPIODeviceNumber.
 * @param   buffer Pointer to store the read value.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eGPIODeviceNumber
 * @retval  eSTATUS_NULL_PARAM    buffer is NULL
 * @retval  eSTATUS_DEVICE_ERROR  device not configured as input or read failed
 */
eStatus hal_gpio_read(uint32_t device_index, int* buffer);

/**
 * @brief   Write a value to a GPIO pin.
 * @details Sets the GPIO pin associated with the specified device to
 *          high or low.
 * @param   device_index A value from @ref eGPIODeviceNumber.
 * @param   value The value to write (zero = low, non-zero = high).
 * @returns A value from @ref eHALReturnValues.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eGPIODeviceNumber
 * @retval  eSTATUS_DEVICE_ERROR  device not configured as output or write failed
 */
eStatus hal_gpio_write(uint32_t device_index, int value);

/**
 * @brief   Does nothing. To be implemented later
 */
eStatus hal_gpio_set_direction(void);

/**
 * @brief   Releases the GPIO devices' resources.
 * @details Releases the lines of each GPIO device and closes the GPIO chip.
 */
void hal_gpio_cleanup(void);

#endif