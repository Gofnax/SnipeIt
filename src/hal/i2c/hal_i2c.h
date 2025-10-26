#ifndef HAL_I2C_H
#define HAL_I2C_H

/* Standard Libraries */
#include <stdint.h>

/* Linux Specific Libraries */
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

/* User Libraries */
#include "../hal_types.h"

/**
 * @brief   Sets the I2C devices configuration.
 * @details Sets the I2C devices according to configuration specified
 *          in 'hal_i2c_config.h', and opens them for communication.
 *          For multiple devices that use the same I2C bus, the configuration
 *          of the last one configured sets the address length.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_DEVICE_ERROR        failure to set the device configuration
 * @retval  eRETURN_INVALID_PARAMETER   failure to set bus address length
 */
eHALReturnValue hal_i2c_init(void);

/**
 * @brief   Sets the address length of the I2C device.
 * @details Sets the device address to 7bit or 10bit.
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   addr_len Either zero for 7bit address or a non-zero value 
 *          for 10bit address.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber 
 * @retval  eRETURN_DEVICE_ERROR        failure to read from device or it doesn't support 10bit address
 */
eHALReturnValue hal_i2c_set_address_length(uint32_t device_index, uint8_t addr_len);

/**
 * @brief   Sets the address of the I2C device.
 * @details Sets the address used by the indicated device on the I2C bus
 *          to which it's connected.
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   address A value unique to this device (usually between 0x08-0x77).
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber
 */
eHALReturnValue hal_i2c_set_address(uint32_t device_index, uint16_t address);

/**
 * @brief   Transfers an array of I2C messages.
 * @details Using the i2c_msg struct, transfers a number of messages specified by count.
 *          Each message can be either a reading or writing message. Only 1 stop bit is
 *          transmitted after all the messages have been transfered.
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   messages A pointer to an array of i2c_msg, representing the messages.
 * @param   count The number of messages pointed to by the array. Must not be greater
 *          than I2C_RDWR_IOCTL_MAX_MSGS.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER      messages is NULL
 * @retval  eRETURN_INVALID_PARAMETER   count is out of range
 * @retval  eRETURN_DEVICE_ERROR        failure to transfer the messages
 */
eHALReturnValue hal_i2c_transfer(uint32_t device_index, const struct i2c_msg* messages, size_t count);

/**
 * @brief   Writes the contents of buffer to an I2C device.
 * @details Assuming the data in buffer begins with the address of the desired register,
 *          performs a full transaction of according to the I2C message protocol [address + W bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   buffer A pointer to the transmit buffer.
 * @param   num_bytes The number of bytes to write to the device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER      buffer is NULL
 * @retval  eRETURN_DEVICE_ERROR        failure to transfer the messages
 */
eHALReturnValue hal_i2c_write(uint32_t device_index, const void* buffer, size_t num_bytes);

/**
 * @brief   Writes the contents of buffer to an I2C device.
 * @details Assuming the data in buffer doesn't begin with the address of the desired
 *          register, requires explicit register address and length. Performs a full
 *          transaction of according to the I2C message protocol [address + W bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   reg The register to be written to.
 * @param   reg_len The length in bits of the register
 * @param   buffer A pointer to the transmit buffer.
 * @param   num_bytes The number of bytes to write to the device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER      buffer is NULL
 * @retval  eRETURN_DEVICE_ERROR        failure to transfer the messages
 */
eHALReturnValue hal_i2c_write_reg(uint32_t device_index, uint16_t reg, size_t reg_len, const void* buffer, size_t num_bytes);

/**
 * @brief   Reads from an I2C device.
 * @details Assuming the data in buffer begins with the address of the desired register,
 *          performs a full transaction of according to the I2C message protocol [address + R bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   buffer A pointer to the receive buffer.
 * @param   num_bytes The number of bytes to read from the device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER      buffer is NULL
 * @retval  eRETURN_DEVICE_ERROR        failure to transfer the messages
 */
eHALReturnValue hal_i2c_read(uint32_t device_index, void* buffer, size_t num_bytes);

/**
 * @brief   Reads from an I2C device.
 * @details Assuming the data in buffer doesn't begin with the address of the desired
 *          register, requires explicit register address and length. Performs a full
 *          transaction of according to the I2C message protocol [address + R bit][data].
 * @param   device_index A value from @ref eI2CDeviceNumber.
 * @param   reg The register to be read from.
 * @param   reg_len The length in bits of the register
 * @param   buffer A pointer to the receive buffer.
 * @param   num_bytes The number of bytes to read from the device.
 * @returns A value from @ref eHALReturnValues.
 * @retval  eRETURN_SUCCESS             successful execution
 * @retval  eRETURN_INVALID_DEVICE      device is not from @ref eI2CDeviceNumber
 * @retval  eRETURN_NULL_PARAMETER      buffer is NULL
 * @retval  eRETURN_DEVICE_ERROR        failure to transfer the messages
 */
eHALReturnValue hal_i2c_read_reg(uint32_t device_index, uint16_t reg, size_t reg_len, void* buffer, size_t num_bytes);

#endif