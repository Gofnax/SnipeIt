#include "hal_i2c.h"

/* Standard Libraries */
#include <stddef.h>

/* Linux Specific Libraries */
#include <sys/ioctl.h>      // The main communication is implemented using the `ioctl()` function
#include <unistd.h>
#include <fcntl.h>

/* User Libraries */
#include "hal_i2c_config.h"

typedef struct
{
    char*       path;       /** Path to the I2C device, e.g. "/dev/i2c-1" */
    int         fd;         /** File descriptor for the I2C device */
    uint16_t    flags;      /** Flags determining the functionality of the messages handled by the device  */
    uint16_t    address;    /** Slave address used to for communication (normally in range 0x08 - 0x77 for 7 bit addresses)*/
} I2CDevice;

static I2CDevice i2c_devices[eI2C_DEVICE_COUNT] = {
    [eI2C0_DEVICE] = {
        .path = "/dev/i2c-1",
        .fd = -1,
        .flags = eI2C0_BITS_PER_ADDRESS_CONFIG,
        .address = eI2C0_ADDRESS_CONFIG
    }
};

eHALReturnValue hal_i2c_init(void)
{
    for(uint32_t device_index = 0; device_index < eI2C_DEVICE_COUNT; ++device_index)
    {
        i2c_devices[device_index].fd = open(i2c_devices[device_index].path, O_RDWR);
        if(i2c_devices[device_index].fd == NULL)
        {
            return eRETURN_DEVICE_ERROR;
        }

        uint32_t funcs = 0;
        // This call to `ioctl` saves in `funcs` a bitmask indicating the device's supported operations
        ioctl(i2c_devices[device_index].fd, I2C_FUNCS, &funcs);
        // We check to see if the device even supports read and write operations (using the I2C_RDWR operation)
        if(!(funcs & I2C_FUNC_I2C))
        {
            return eRETURN_DEVICE_ERROR;
        }

        // Sets the slave address that will be used for read/write operations
        ioctl(i2c_devices[device_index].fd, I2C_SLAVE, i2c_devices[device_index].address);
    }
}