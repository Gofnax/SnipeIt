#ifndef HAL_I2C_H
#define HAL_I2C_H

/* Standard library includes */
#include <stdint.h>
#include <stddef.h>

/* User library includes */
#include "status.h"

/**
 * @brief   Sets the I2C devices configuration.
 * @details Sets the I2C devices according to configuration specified
 *          in 'hal_i2c_config.h', and opens them for communication.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL   successful execution
 * @retval  eSTATUS_DEVICE_ERROR failure to set the device configuration
 */
eStatus hal_i2c_init(void);

/**
 * @brief   Sets the address of the I2C device.
 * @details Sets the address used by the indicated device on the I2C bus
 *          to which it's connected.
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   address A value unique to this device (usually between 0x08-0x77).
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eI2CDeviceNumber
 */
eStatus hal_i2c_set_address(uint32_t device_index, uint8_t address);

/**
 * @brief   Writes the contents of buffer to an I2C device.
 * @details Assuming the data in buffer begins with the address of the desired register,
 *          performs a full transaction according to the I2C message protocol [address + W bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   buffer A pointer to the transmit buffer.
 * @param   num_bytes The number of bytes to write to the device.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eI2CDeviceNumber
 * @retval  eSTATUS_NULL_PARAM    buffer is NULL
 * @retval  eSTATUS_DEVICE_ERROR  failure to transfer the messages
 */
eStatus hal_i2c_write(uint32_t device_index, void* buffer, size_t num_bytes);

/**
 * @brief   Writes the contents of buffer to an I2C device.
 * @details Assuming the data in buffer doesn't begin with the address of the desired
 *          register, requires explicit register address and length. Performs a full
 *          transaction of according to the I2C message protocol [address + W bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   reg The register to be written to.
 * @param   reg_len The length in bytes of the register
 * @param   buffer A pointer to the transmit buffer.
 * @param   num_bytes The number of bytes to write to the device.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eI2CDeviceNumber
 * @retval  eSTATUS_NULL_PARAM    buffer is NULL
 * @retval  eSTATUS_DEVICE_ERROR  failure to transfer the messages
 */
eStatus hal_i2c_write_reg(uint32_t device_index, uint16_t reg, size_t reg_len, void* buffer, size_t num_bytes);

/**
 * @brief   Reads from an I2C device.
 * @details Assuming the data in buffer begins with the address of the desired register,
 *          performs a full transaction according to the I2C message protocol [address + R bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   buffer A pointer to the receive buffer.
 * @param   num_bytes The number of bytes to read from the device.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eI2CDeviceNumber
 * @retval  eSTATUS_NULL_PARAM    buffer is NULL
 * @retval  eSTATUS_DEVICE_ERROR  failure to transfer the messages
 */
eStatus hal_i2c_read(uint32_t device_index, void* buffer, size_t num_bytes);

/**
 * @brief   Reads from an I2C device.
 * @details Assuming the data in buffer doesn't begin with the address of the desired
 *          register, requires explicit register address and length. Performs a full
 *          transaction of according to the I2C message protocol [address + R bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   reg The register to be read from.
 * @param   reg_len The length in bytes of the register
 * @param   buffer A pointer to the receive buffer.
 * @param   num_bytes The number of bytes to read from the device.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL    successful execution
 * @retval  eSTATUS_INVALID_VALUE device is not from @ref eI2CDeviceNumber
 * @retval  eSTATUS_NULL_PARAM    buffer is NULL
 * @retval  eSTATUS_DEVICE_ERROR  failure to transfer the messages
 */
eStatus hal_i2c_read_reg(uint32_t device_index, uint16_t reg, size_t reg_len, void* buffer, size_t num_bytes);

// TODO: document
void hal_i2c_cleanup(void);

#endif