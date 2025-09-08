#ifndef HAL_UART_H
#define HAL_UART_H

typedef struct
{
    char*       path;           // Path to the UART device, e.g. "/dev/ttyAMA0"
    int         fd;             // File descriptor for the UART device
    speed_t     baud;           // Baud rate or BPS for the device communication
    tcflag_t    word_size;      // 
    uint8_t     stop_bits;
    uint8_t     parity;
} UARTDevice;

int main(void);

#endif