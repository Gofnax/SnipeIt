#ifndef HAL_UART_H
#define HAL_UART_H

#include "../hal_types.h"
#include <stdint.h>
#include <stddef.h>

/**
 * @brief   Sets the UART devices configuration.
 * @details Sets the UART devices according to configuration specified
 *          in 'hal_uart_config.h'. and opens them for communication,
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS         successful execution
 * @retval  eRETURN_DEVICE_ERROR    failure to set the device configuration
 */
eHALReturnValue hal_uart_init(void);

/**
 * @brief   Read in blocking mode.
 * @details Reads 'count' bytes from the device associated with 'device_index'
 *          into 'buf'.
 * @param   device_index A value from @ref eUARTDeviceNumber.
 * @param   buf A pointer to the receive buffer.
 * @param   count The number of bytes to read from the device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS         successful execution
 * @retval  eRETURN_INVALID_DEVICE  device is not from @ref eUARTDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER  buf is NULL
 * @retval  eRETURN_DEVICE_ERROR    internal read failed
 */
eHALReturnValue hal_uart_read(uint32_t device_index, void* buf, size_t count);

/**
 * @brief   Write in blocking mode.
 * @details Writes 'count' bytes to the device associated with 'device_index'
 *          from 'buf'.
 * @param   device_index A value from @ref eUARTDeviceNumber.
 * @param   buf A pointer to the transmit buffer.
 * @param   count The number of bytes to written to the device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS         successful execution
 * @retval  eRETURN_INVALID_DEVICE  device is not from @ref eUARTDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER  buf is NULL
 */
eHALReturnValue hal_uart_write(uint32_t device_index, const void* buf, size_t count);

#endif