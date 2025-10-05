#include "hal_i2c.h"

/* Standard Libraries */
#include <stddef.h>

/* Linux Specific Libraries */
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

/* User Libraries */
#include "hal_i2c_config.h"

typedef struct
{
    char*       path;       /** Path to the I2C device, e.g. "/dev/i2c-1" */
    int         fd;         /** File descriptor for the I2C device */
    uint16_t    flags;      /** Flags determining I2C device functionality  */
} I2CDevice;

static I2CDevice i2c_devices[eI2C_DEVICE_COUNT] = {

};