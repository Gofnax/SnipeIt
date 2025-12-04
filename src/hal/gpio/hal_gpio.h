#ifndef HAL_GPIO_H
#define HAL_GPIO_H

/* Standard Libraries */
#include <stdint.h>

/* User Libraries */
#include "../hal_types.h"

/**
 * @brief   Sets the GPIO devices configuration.
 * @details Opens the GPIO chip and configures all GPIO devices according to
 *          configuration specified in 'hal_gpio_config.h'. Requests and
 *          configures GPIO lines for each device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_PARAMETER   failure to define pull or edge configuration
 * @retval  eRETURN_DEVICE_ERROR        failure to open chip or configure lines
 */
eHALReturnValue hal_gpio_init(void);

/**
 * @brief   Read the value of a GPIO pin.
 * @details Reads the current value (high/low) from the GPIO pin associated
 *          with the specified device.
 * @param   device_index A value from @ref eGPIODeviceNumber.
 * @param   buffer Pointer to store the read value.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS         successful execution
 * @retval  eRETURN_INVALID_DEVICE  device is not from @ref eGPIODeviceNumber
 * @retval  eRETURN_NULL_PARAMETER  buffer is NULL
 * @retval  eRETURN_DEVICE_ERROR    device not configured as input or read failed
 */
eHALReturnValue hal_gpio_read(uint32_t device_index, int* buffer);

/**
 * @brief   Write a value to a GPIO pin.
 * @details Sets the GPIO pin associated with the specified device to
 *          high or low.
 * @param   device_index A value from @ref eGPIODeviceNumber.
 * @param   value The value to write (zero = low, non-zero = high).
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS         successful execution
 * @retval  eRETURN_INVALID_DEVICE  device is not from @ref eGPIODeviceNumber
 * @retval  eRETURN_DEVICE_ERROR    device not configured as output or write failed
 */
eHALReturnValue hal_gpio_write(uint32_t device_index, int value);

/**
 * @brief   Does nothing. To be implemented later
 */
eHALReturnValue hal_gpio_set_direction(void);

#endif