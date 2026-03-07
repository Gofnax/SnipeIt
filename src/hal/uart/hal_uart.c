#include "hal_uart.h"

eStatus hal_uart_init(void)
{
    return eSTATUS_SUCCESSFUL;
}

eStatus hal_uart_write(uint32_t device, const void* buf, size_t len, async_cb callback, void* arg)
{
    (void)device;
    (void)buf;
    (void)len;
    (void)callback;
    (void)arg;
    return eSTATUS_SUCCESSFUL;
}

eStatus hal_uart_read(uint32_t device, void* buf, size_t len, async_cb callback, void* arg)
{
    (void)device;
    (void)buf;
    (void)len;
    (void)callback;
    (void)arg;
    return eSTATUS_SUCCESSFUL;
}

eStatus hal_uart_abort(uint32_t device)
{
    (void)device;
    return eSTATUS_SUCCESSFUL;
}