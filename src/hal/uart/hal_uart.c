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

#define NUM_UART_DEVICES 3

typedef struct
{
    int         fd;             /** File descriptor for the UART device */
    char*       path;           /** Path to the UART device, e.g. "/dev/ttyAMA0" */
    speed_t     baud;           /** Baud rate or BPS for the device communication */
    tcflag_t    word_size;      /** Word size (5-8 bits) */
    uint8_t     stop_bits;      /** Single or Double stop bits */
    uint8_t     parity;         /** Parity bits in use (None, Even, or Odd) */
} UARTDevice;

static UARTDevice uart_devices[NUM_UART_DEVICES];

int hal_uart_init()
{
    const speed_t baud_options[] = {
        B0, B50, B75, B110, B134,
        B150, B200, B300, B600, B1200,
        B1800, B2400, B4800, B9600, B19200,
        B38400, B57600, B115200, B230400, B460800
    };

    const tcflag_t word_size_options[] = { CS5, CS6, CS7, CS8 };

    const uint8_t stop_bits_options[] = { 0, 1 };      // relates to CSTOPB flag; 0 - single bit; 1 - double bits

    const uint8_t parity_options[] = { 0, 1, 2 };      // relates to PARENB and PARODD flags; 0 - none; 1 - even; 2 - odd

    uart_devices[eUART0_DEVICE].fd = 0;
    uart_devices[eUART0_DEVICE].path = "/dev/ttyAMA0";
    uart_devices[eUART0_DEVICE].baud = baud_options[eUART0_BAUD_CONFIG];
    uart_devices[eUART0_DEVICE].word_size = word_size_options[eUART0_BITS_PER_BYTE_CONFIG];
    uart_devices[eUART0_DEVICE].stop_bits = stop_bits_options[eUART0_STOP_BIT_CONFIG];
    uart_devices[eUART0_DEVICE].parity = parity_options[eUART0_PARITY_BIT_CONFIG];

    uart_devices[eUART1_DEVICE].fd = 0;
    uart_devices[eUART1_DEVICE].path = "/dev/ttyAMA1";
    uart_devices[eUART1_DEVICE].baud = baud_options[eUART1_BAUD_CONFIG];
    uart_devices[eUART1_DEVICE].word_size = word_size_options[eUART1_BITS_PER_BYTE_CONFIG];
    uart_devices[eUART1_DEVICE].stop_bits = stop_bits_options[eUART1_STOP_BIT_CONFIG];
    uart_devices[eUART1_DEVICE].parity = parity_options[eUART1_PARITY_BIT_CONFIG];

    uart_devices[eUART2_DEVICE].fd = 0;
    uart_devices[eUART2_DEVICE].path = "/dev/ttyAMA2";
    uart_devices[eUART2_DEVICE].baud = baud_options[eUART2_BAUD_CONFIG];
    uart_devices[eUART2_DEVICE].word_size = word_size_options[eUART2_BITS_PER_BYTE_CONFIG];
    uart_devices[eUART2_DEVICE].stop_bits = stop_bits_options[eUART2_STOP_BIT_CONFIG];
    uart_devices[eUART2_DEVICE].parity = parity_options[eUART2_PARITY_BIT_CONFIG];

    // Here we set the baud, flags (we don't need any), word size, stop bits, and parity bits for each UART device
    // All the bits configuration is done via the termios struct with its flags and the 'tcsetattr' function
    // The baud is set using 'cfsetospeed' and 'cfsetispeed' functions
    // We get the file descriptor for each device after we configure it and use the POSIX 'open' function

    struct termios temp_config = { 0 };

    temp_config.c_cflag |= CLOCAL | CREAD; // Prevents waiting for modem connection and enables reading input from the terminal

    temp_config.c_cflag |= uart_devices[eUART0_DEVICE].word_size;   // Sets the bits per byte

    if(uart_devices[eUART0_DEVICE].stop_bits == 0)
    {
       temp_config.c_cflag &= (tcflag_t)~(CSTOPB);  // Ensures CSTOPB bit is 0  (sets use of one stop bit)
    }
    else
    {
        temp_config.c_cflag |= CSTOPB;              // Sets CSTOPB bit to 1 (sets use of two stop bits)
    }

    if(uart_devices[eUART0_DEVICE].parity == 0)
    {
        temp_config.c_cflag &= (tcflag_t)~(PARENB);     // Ensures PAREND bit is 0 (disables parity generation and detection)
        temp_config.c_iflag &= (tcflag_t)~(INPCK);      // Ensures INPCK bit is 0 (disables input parity checking)
    }
    else
    {
        temp_config.c_cflag |= PARENB;                  // Sets PAREND bit to 1 (enables parity generation and detection)
        if(uart_devices[eUART0_DEVICE].parity == 1)
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

    
}