#ifndef DDL_I2C_CONFIG_H
#define DDL_I2C_CONFIG_H

typedef enum eI2CBitsPerAddress
{
    e7BITS_PER_ADDRESS,
    e10BITS_PER_ADDRESS
} eI2CBitsPerAddress;

typedef enum eI2CDeviceNumber
{
    eI2C0_DEVICE,
    eI2C_DEVICE_COUNT
} eI2CDeviceNumber;

typedef enum eI2CAddress
{
    eI2C0_ADDRESS = 0x08    // Normally in range 0x08-0x77 for 7 bit addresses
} eI2CAddress;

typedef enum eI2CConfig
{
    eI2C0_BITS_PER_ADDRESS_CONFIG   = e7BITS_PER_ADDRESS,
    eI2C0_ADDRESS_CONFIG            = eI2C0_ADDRESS
} eI2CConfig;

#endif