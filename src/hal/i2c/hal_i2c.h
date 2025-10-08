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

#endif