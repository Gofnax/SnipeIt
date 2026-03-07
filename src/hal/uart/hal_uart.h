#ifndef HAL_UART_H
#define HAL_UART_H

/* Standard library includes */
#include <stdint.h>
#include <stddef.h>

/* User library includes */
#include "status.h"

typedef void (*async_cb)(void *user_data);

eStatus hal_uart_init(void);

eStatus hal_uart_write(uint32_t device, const void* buf, size_t len, async_cb callback, void* arg);

eStatus hal_uart_read(uint32_t device, void* buf, size_t len, async_cb callback, void* arg);

eStatus hal_uart_abort(uint32_t device);

#endif