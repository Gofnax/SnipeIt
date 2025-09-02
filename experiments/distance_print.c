#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../src/uart.h"

int main(void)
{
    UARTDevice uart2 = {
		.pname     = "/dev/ttyAMA0",
		.baud      = B9600,
		.bits      = CS8,
		.stop_bits = SINGLE_STOP_BIT,
		.parity    = PARITY_NONE,
		.vmin      = 1,
		.vtime     = SEC_TO_VTIME(10)
	};

    struct timespec ts = {1, 0};    //{0, 500000000};

    printf("Opening UART device %s\n", uart2.pname);
	if(uart_open(&uart2) != eUART_OK)
	{
		perror("Failed to open uart");
		exit(EXIT_FAILURE);
	}

    printf("Setting configuration...\n");
	if(uart_set_configuration(&uart2) != eUART_OK)
	{
		perror("Failed to configure uart");
		uart_close(&uart2);
		exit(EXIT_FAILURE);
	}

    UARTDevice uart2_check = {
		.pname = uart2.pname,
		.fd    = uart2.fd
	};

    printf("Reading configuration...\n");
	if(uart_get_configuration(&uart2_check) != eUART_OK)
	{
		perror("Failed to read configuration");
		uart_close(&uart2);
		exit(EXIT_FAILURE);
	}

    printf("Comparing uart2 and uart2_check...\n");
	if(memcmp(&uart2, &uart2_check, sizeof(UARTDevice)) != 0)
	{
		perror("Comparison failed! uart2 and uart2_check are not the same.\n");
	}
	printf("Success! uart2 and uart2_check are the same\n");

	const uint8_t command[] = { 0x57, 0x10, 0xff, 0xff, 0x00, 0xff, 0xff, 0x63 };
	uint8_t read_buff[16];
	size_t cmd_len = sizeof(command);
	size_t read_len = sizeof(read_buff);
	while(1)
	{
		uart_write_all(&uart2, command, &cmd_len);
		uart_read_all(&uart2, read_buff, &read_len);
		for(int i = 0; i < 16; i++)
		{
            printf("%02X ", read_buff[i]);
        }
		printf("\n");
		
		if(read_buff[0] == 0x57 && read_buff[1] == 0x00)
		{
			float dist = ((int32_t)((int32_t)(read_buff[8]) << 8 | (int32_t)(read_buff[9]) << 16 | (int32_t)(read_buff[10]) << 24) / 256) / 1000.0f;
			printf("Distance: %f\n", dist);
		}
		nanosleep(&ts, NULL);
		cmd_len = sizeof(command);
		read_len = sizeof(read_buff);
	}

	#if 0
    printf("Sampling dinstace:\n");
    while(1)
    {
        uint8_t recv_buf[16];   // size used in sample code
        size_t len = sizeof(recv_buf);
        UARTStatus status = uart_read_all(&uart2, recv_buf, &len);
        if(status == eUART_READ_ERR)
        {
            perror("Failed to read");
            uart_close(&uart2);
            exit(EXIT_FAILURE);
        }
        else if(status == eUART_READ_TIMEOUT)
        {
            perror("Read timed out!\n");
            uart_close(&uart2);
            exit(EXIT_FAILURE);
        }

		// Print raw values for debugging
        printf("Raw bytes: ");
        for(int i = 0; i < 16; i++)
		{
            printf("%02X ", recv_buf[i]);
        }
		printf("\n%02X %02X %02X\n", recv_buf[8], recv_buf[9], recv_buf[10]);

		if(recv_buf[0] == 0x57 && recv_buf[1] == 0x00)
		{
			float dist = ((int32_t)((int32_t)(recv_buf[8]) << 8 | (int32_t)(recv_buf[9]) << 16 | (int32_t)(recv_buf[10]) << 24) / 256) / 1000.0f;
			printf("Distance: %f\n", dist);
			nanosleep(&ts, NULL);
		}
    }
	#endif
    uart_close(&uart2);
	return 0;
}