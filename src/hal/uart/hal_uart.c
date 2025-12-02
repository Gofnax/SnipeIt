#include "hal_uart.h"

/* Standard Libraries */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <sys/poll.h>
#include <errno.h>

/* Linux Specific Libraries */
#include <termios.h>

/* User Libraries */
#include "hal_uart_config.h"

typedef struct
{
    char*       path;           /** Path to the UART device, e.g. "/dev/ttyAMA0" */
    int         fd;             /** File descriptor for the UART device */
    speed_t     baud;           /** Baud rate or BPS for the device communication */
    tcflag_t    word_size;      /** Word size (5-8 bits) */
    uint16_t    stop_bits;      /** Single or Double stop bits */
    uint16_t    parity;         /** Parity bits in use (None, Even, or Odd) */
} UARTDevice;

static UARTDevice uart_devices[eUART_DEVICE_COUNT] = {
    [eUART0_DEVICE] = {
        .path = "/dev/ttyAMA0",
        .fd = -1,
        .baud = eUART0_BAUD_CONFIG,
        .word_size = eUART0_BITS_PER_BYTE_CONFIG,
        .stop_bits = eUART0_STOP_BIT_CONFIG,
        .parity = eUART0_PARITY_BIT_CONFIG
    },
    [eUART1_DEVICE] = {
        .path = "/dev/ttyAMA1",
        .fd = -1,
        .baud = eUART1_BAUD_CONFIG,
        .word_size = eUART1_BITS_PER_BYTE_CONFIG,
        .stop_bits = eUART1_STOP_BIT_CONFIG,
        .parity = eUART1_PARITY_BIT_CONFIG
    },
    [eUART2_DEVICE] = {
        .path = "/dev/ttyAMA2",
        .fd = -1,
        .baud = eUART2_BAUD_CONFIG,
        .word_size = eUART2_BITS_PER_BYTE_CONFIG,
        .stop_bits = eUART2_STOP_BIT_CONFIG,
        .parity = eUART2_PARITY_BIT_CONFIG
    }
};

eHALReturnValue hal_uart_init(void)
{
    const speed_t baud_options[] = {
        B0, B50, B75, B110, B134,
        B150, B200, B300, B600, B1200,
        B1800, B2400, B4800, B9600, B19200,
        B38400, B57600, B115200, B230400, B460800
    };

    const tcflag_t word_size_options[] = { CS5, CS6, CS7, CS8 };

    // Here we set the baud, flags (we don't need any), word size, stop bits, and parity bits for each UART device
    // All the bits configuration is done via the termios struct with its flags and the 'tcsetattr' function
    // The baud is set using 'cfsetospeed' and 'cfsetispeed' functions and the termios struct

    for(uint32_t device_index = 0; device_index < eUART_DEVICE_COUNT; ++device_index)
    {
        struct termios temp_config = { 0 };

        // Prevents waiting for modem connection and enables reading input from the terminal
        temp_config.c_cflag = CLOCAL | CREAD;

        // Sets the bits per byte
        temp_config.c_cflag |= word_size_options[uart_devices[device_index].word_size];

        // Determine stop bit count
        if(uart_devices[device_index].stop_bits == eSINGLE_STOP_BIT)
        {
            // Ensures CSTOPB bit is 0  (sets use of one stop bit)
            temp_config.c_cflag &= (tcflag_t)~(CSTOPB);
        }
        else
        {
            // Sets CSTOPB bit to 1 (sets use of two stop bits)
            temp_config.c_cflag |= CSTOPB;
        }

        // Determine parity bit 
        if(uart_devices[device_index].parity == eNO_PARITY_BIT)
        {
            temp_config.c_cflag &= (tcflag_t)~(PARENB);     // Ensures PAREND bit is 0 (disables parity generation and detection)
            temp_config.c_iflag &= (tcflag_t)~(INPCK);      // Ensures INPCK bit is 0 (disables input parity checking)
        }
        else
        {
            temp_config.c_cflag |= PARENB;                  // Sets PAREND bit to 1 (enables parity generation and detection)
            if(uart_devices[device_index].parity == eEVEN_PARITY_BIT)
            {
                temp_config.c_cflag &= (tcflag_t)~(PARODD); // Ensures PARODD bit is 0 (sets parity to even)
            }
            else
            {
                temp_config.c_cflag |= PARODD;              // Sets PARODD bit to 1 (sets parity to odd)
            }
            temp_config.c_iflag |= INPCK;                   // Sets INPCK bit to 1 (enables input parity checking)
            temp_config.c_iflag |= IGNPAR;                  // Sets IGNPAR bit to 1 (sets framing or parity errors to be ignored)
        }

        // These function return 0 if all went normal and -1 if an error occured
        if(cfsetispeed(&temp_config, baud_options[uart_devices[device_index].baud]) < 0 || 
            cfsetospeed(&temp_config, baud_options[uart_devices[device_index].baud]) < 0)       
        {
            return eRETURN_DEVICE_ERROR;
        }

        uart_devices[device_index].fd = open(uart_devices[device_index].path, O_RDWR | O_NOCTTY);
        if(uart_devices[device_index].fd < 0)
        {
            return eRETURN_DEVICE_ERROR;
        }

        if(!isatty(uart_devices[device_index].fd))
        {
            close(uart_devices[device_index].fd);
            return eRETURN_DEVICE_ERROR;
        }

        if(tcsetattr(uart_devices[device_index].fd, TCSAFLUSH, &temp_config))
        {
            return eRETURN_DEVICE_ERROR;
        }
    }

    return eRETURN_SUCCESS;
}

eHALReturnValue hal_uart_read(uint32_t device_index, void* buffer, size_t num_bytes)
{
    if(device_index >= eUART_DEVICE_COUNT)
    {
        return eRETURN_INVALID_DEVICE;
    }

    if(buffer == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }

    size_t bytes_read = 0;
    while(num_bytes > 0)
    {
        bytes_read = read(uart_devices[device_index].fd, buffer, num_bytes);
        if(bytes_read < 0)
        {
            //These errors allow retry of reading
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                return eRETURN_DEVICE_ERROR;
            }
        }
        buffer += bytes_read;
        num_bytes -= bytes_read;
    }

    return eRETURN_SUCCESS;
}

eHALReturnValue hal_uart_write(uint32_t device_index, const void* buffer, size_t num_bytes)
{
    if(device_index >= eUART_DEVICE_COUNT)
    {
        return eRETURN_INVALID_DEVICE;
    }

    if(buffer == NULL)
    {
        return eRETURN_NULL_PARAMETER;
    }

    size_t bytes_written = 0;
    while(num_bytes > 0)
    {
        bytes_written = write(uart_devices[device_index].fd, buffer, num_bytes);
        if(bytes_written < 0)
        {
            //These errors allow retry of reading
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                return eRETURN_INVALID_DEVICE;
            }
        }
        buffer += bytes_written;
        num_bytes -= bytes_written;
    }

    return eRETURN_SUCCESS;
}