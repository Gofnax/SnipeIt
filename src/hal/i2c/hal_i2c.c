#include "hal_i2c.h"

/* Standard Libraries */
#include <stddef.h>

/* Linux Specific Libraries */
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>      // The main communication is implemented using the `ioctl()` function
#include <unistd.h>
#include <fcntl.h>

#define I2C_SINGLE_MESSAGE 1
#define I2C_DOUBLE_MESSAGE 2

typedef struct
{
    char*       path;       /** Path to the I2C bus, e.g. "/dev/i2c-1" */
    int         fd;         /** File descriptor for the I2C device */
    uint16_t    flags;      /** Flags determining the functionality of the messages handled by the device  */
    uint8_t    address;    /** Slave address used to for communication */
} I2CDevice;

static I2CDevice i2c_devices[eI2C_DEVICE_COUNT] = {
    [eI2C0_DEVICE] = {
        .path = "/dev/i2c-1",
        .fd = -1,
        .flags = 0,      // Possible flag is `I2C_M_TEN` for 10bit address length (defualt is 7bit)
        .address = 0
    }
};

static eHALReturnValue hal_i2c_transfer(uint32_t device_index, struct i2c_msg* messages, size_t count)
{
    if(device_index > eI2C_DEVICE_COUNT)
    {
        return eRETURN_INVALID_DEVICE;
    }
    if(messages == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }
    if(count > I2C_RDWR_IOCTL_MAX_MSGS)
    {
        return eRETURN_INVALID_PARAMETER;
    }
    
    struct i2c_rdwr_ioctl_data transfer = {
        .msgs = messages,
        .nmsgs = count
    };

    if(ioctl(i2c_devices[device_index].fd, I2C_RDWR, &transfer) < 0)
    {
        return eRETURN_DEVICE_ERROR;
    }

    return eRETURN_SUCCESS;
}

eHALReturnValue hal_i2c_init(void)
{
    for(uint32_t device_index = 0; device_index < eI2C_DEVICE_COUNT; ++device_index)
    {
        i2c_devices[device_index].fd = open(i2c_devices[device_index].path, O_RDWR);
        if(i2c_devices[device_index].fd < 0)
        {
            return eRETURN_DEVICE_ERROR;
        }

        uint32_t funcs = 0;
        // This call to `ioctl` saves in `funcs` a bitmask indicating the device's supported operations
        if(ioctl(i2c_devices[device_index].fd, I2C_FUNCS, &funcs) < 0)
        {
            return eRETURN_DEVICE_ERROR;
        }
        // We check to see if the device even supports read and write operations (using the I2C_RDWR operation)
        if(!(funcs & I2C_FUNC_I2C))
        {
            return eRETURN_DEVICE_ERROR;
        }

        // The PWM MUX uses 7bit address
        i2c_devices[device_index].flags &= (uint16_t)~(I2C_M_TEN);
    }

    return eRETURN_SUCCESS;
}

eHALReturnValue hal_i2c_set_address(uint32_t device_index, uint8_t address)
{
    if(device_index > eI2C_DEVICE_COUNT)
    {
        return eRETURN_INVALID_DEVICE;
    }

    i2c_devices[device_index].address = address;

    return eRETURN_SUCCESS;
}

eHALReturnValue hal_i2c_write(uint32_t device_index, void* buffer, size_t num_bytes)
{
    if(buffer == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }

    struct i2c_msg messages[I2C_SINGLE_MESSAGE] = {
        {
            .addr = i2c_devices[device_index].address,
            .flags = i2c_devices[device_index].flags,
            .len = num_bytes,
            .buf = buffer
        }
    };

    return hal_i2c_transfer(device_index, messages, I2C_SINGLE_MESSAGE);
}

eHALReturnValue hal_i2c_write_reg(uint32_t device_index, uint16_t reg, size_t reg_len, void* buffer, size_t num_bytes)
{
    if(buffer == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }

    struct i2c_msg messages[I2C_DOUBLE_MESSAGE] = {
        {
            .addr = i2c_devices[device_index].address,
            .flags = i2c_devices[device_index].flags,
            .len = reg_len,
            .buf = (uint8_t*)&reg
        },
        {
            .addr = i2c_devices[device_index].address,
            .flags = i2c_devices[device_index].flags,
            .len = num_bytes,
            .buf = buffer
        }
    };

    return hal_i2c_transfer(device_index, messages, I2C_DOUBLE_MESSAGE);
}

eHALReturnValue hal_i2c_read(uint32_t device_index, void* buffer, size_t num_bytes)
{
    if(buffer == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }

    struct i2c_msg message = {
        .addr = i2c_devices[device_index].address,
        .flags = i2c_devices[device_index].flags | I2C_M_RD,    // We add the READ bit to signify a read message
        .len = num_bytes,
        .buf = buffer
    };

    return hal_i2c_transfer(device_index, &message, I2C_SINGLE_MESSAGE);
}

eHALReturnValue hal_i2c_read_reg(uint32_t device_index, uint16_t reg, size_t reg_len, void* buffer, size_t num_bytes)
{
    if(buffer == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }

    struct i2c_msg messages[I2C_DOUBLE_MESSAGE] = {
        {
            .addr = i2c_devices[device_index].address,
            .flags = i2c_devices[device_index].flags,
            .len = reg_len,
            .buf = (uint8_t*)&reg
        },
        {
            .addr = i2c_devices[device_index].address,
            .flags = i2c_devices[device_index].flags | I2C_M_RD,
            .len = num_bytes,
            .buf = buffer
        }
    };

    return hal_i2c_transfer(device_index, messages, I2C_DOUBLE_MESSAGE);
}