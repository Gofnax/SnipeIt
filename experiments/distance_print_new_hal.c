/* User Libraries */
#include "../src/hal/uart/hal_uart.h"
#include "../src/hal/uart/hal_uart_config.h"

/* Standard Libraries */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Linux Specific Libraries */
#include <termios.h>

#define DISTANCE_SENSOR_INDEX 0

int main(void)
{
    eHALReturnValue ret_val = hal_uart_init();
    if(ret_val != eRETURN_SUCCESS)
    {
        perror("UART initiation failed");
        return 1;
    }

    const uint8_t command[] = { 0x57, 0x10, 0xff, 0xff, 0x00, 0xff, 0xff, 0x63 };
	uint8_t read_buff[16];
	struct timespec ts = {1, 0};

    while(1)
    {
        ret_val = hal_uart_write(DISTANCE_SENSOR_INDEX, command, sizeof(command));
        if(ret_val != eRETURN_SUCCESS)
        {
            perror("Couldn't write to UART device");
            return 1;
        }

        ret_val = hal_uart_read(DISTANCE_SENSOR_INDEX, read_buff, sizeof(read_buff));
        if(ret_val != eRETURN_SUCCESS)
        {
            perror("Couldn't read from UART device");
            return 1;
        }
        for(int i = 0; i < 16; ++i)
        {
            printf("%02X ", read_buff[i]);
        }
        printf("\n");

        // First two bytes should be 0x57 and 0x00 if read was successful
        if(read_buff[0] == 0x57 && read_buff[1] == 0x00)
        {
            // The distance parsing fomula provided by the manufacturer
            float dist = ((int32_t)((int32_t)(read_buff[8]) << 8 | (int32_t)(read_buff[9]) << 16 | (int32_t)(read_buff[10]) << 24) / 256) / 1000.0f;
            printf("Distance: %f\n", dist);
        }
        else
        {
            printf("Read wasn't successful\n");
        }
        nanosleep(&ts, NULL);
        
    }
}