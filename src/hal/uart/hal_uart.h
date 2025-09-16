#ifndef HAL_UART_H
#define HAL_UART_H

int hal_uart_init();
int hal_uart_read(int fd, void* buf, size_t count);

#endif