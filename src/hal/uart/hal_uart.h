#ifndef HAL_UART_H
#define HAL_UART_H

/**
 * @brief   Sets the UART devices configuration.
 * @details Sets the UART devices according to configuration specified
 *          in 'hal_uart_config.h'. and opens them for communication,
 * @returns 0 on success. -1 on failure or error.
 */
int hal_uart_init();

/**
 * @brief   Read in blocking mode.
 * @details Reads 'count' bytes from the device associated with 'device_index'
 *          into 'buf'.
 * @param   device_index An index associated with a UART device. This index
 *          specifies which device should be addressed.
 * @param   buf A pointer to the receive buffer.
 * @param   count The number of bytes to read from the device.
 * @returns 0 on success. -1 on failure or error.
 */
int hal_uart_read(int device_index, void* buf, size_t count);

/**
 * @brief   Write in blocking mode.
 * @details Writes 'count' bytes to the device associated with 'device_index'
 *          from 'buf'.
 * @param   device_index An index associated with a UART device. This index
 *          specifies which device should be addressed.
 * @param   buf A pointer to the transmit buffer.
 * @param   count The number of bytes to written to the device.
 * @returns 0 on success. -1 on failure or error. 
 */
int hal_uart_write(int device_index, void* buf, size_t count);

#endif