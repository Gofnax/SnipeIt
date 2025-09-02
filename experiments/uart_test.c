#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "../src/uart.h"

void print_baud(const UARTDevice* pdevice)
{
	char baud_str[] = "ERR    ";
	switch(pdevice->baud)
	{
		case B0:      strncpy(baud_str, "B0     ", sizeof(baud_str) - 1); break;
		case B50:     strncpy(baud_str, "B50    ", sizeof(baud_str) - 1); break;
		case B75:     strncpy(baud_str, "B75    ", sizeof(baud_str) - 1); break;
		case B110:    strncpy(baud_str, "B110   ", sizeof(baud_str) - 1); break;
		case B134:    strncpy(baud_str, "B134   ", sizeof(baud_str) - 1); break;
		case B150:    strncpy(baud_str, "B150   ", sizeof(baud_str) - 1); break;
		case B200:    strncpy(baud_str, "B200   ", sizeof(baud_str) - 1); break;
		case B300:    strncpy(baud_str, "B300   ", sizeof(baud_str) - 1); break;
		case B600:    strncpy(baud_str, "B600   ", sizeof(baud_str) - 1); break;
		case B1200:   strncpy(baud_str, "B1200  ", sizeof(baud_str) - 1); break;
		case B1800:   strncpy(baud_str, "B1800  ", sizeof(baud_str) - 1); break;
		case B2400:   strncpy(baud_str, "B2400  ", sizeof(baud_str) - 1); break;
		case B4800:   strncpy(baud_str, "B4800  ", sizeof(baud_str) - 1); break;
		case B9600:   strncpy(baud_str, "B9600  ", sizeof(baud_str) - 1); break;
		case B19200:  strncpy(baud_str, "B19200 ", sizeof(baud_str) - 1); break;
		case B38400:  strncpy(baud_str, "B38400 ", sizeof(baud_str) - 1); break;
		case B57600:  strncpy(baud_str, "B57600 ", sizeof(baud_str) - 1); break;
		case B115200: strncpy(baud_str, "B115200", sizeof(baud_str) - 1); break;
		case B230400: strncpy(baud_str, "B230400", sizeof(baud_str) - 1); break;
		case B460800: strncpy(baud_str, "B460800", sizeof(baud_str) - 1); break;
	}
	printf("baud:      %s\n", baud_str);
}

void print_bits(const UARTDevice* pdevice)
{
	char bits_str[] = "ERR";
	switch(pdevice->bits)
	{
		case CS5: strncpy(bits_str, "CS5", sizeof(bits_str) - 1); break;
		case CS6: strncpy(bits_str, "CS6", sizeof(bits_str) - 1); break;
		case CS7: strncpy(bits_str, "CS7", sizeof(bits_str) - 1); break;
		case CS8: strncpy(bits_str, "CS8", sizeof(bits_str) - 1); break;
	}
	printf("bits:      %s\n", bits_str);
}

void print_stop_bits(const UARTDevice* pdevice)
{
	char stop_bits_str[] = "ERR            ";
	if(pdevice->stop_bits == DOUBLE_STOP_BIT)
		strncpy(stop_bits_str, "DOUBLE_STOP_BIT", sizeof(stop_bits_str) - 1);
	else if(pdevice->stop_bits == SINGLE_STOP_BIT)
		strncpy(stop_bits_str, "SINGLE_STOP_BIT", sizeof(stop_bits_str) - 1);
	printf("stop bits: %s\n", stop_bits_str);
}

void print_parity(const UARTDevice* pdevice)
{
	char parity_str[] = "ERR        ";
	if(pdevice->parity == PARITY_NONE)
		strncpy(parity_str, "PARITY_NONE", sizeof(parity_str) - 1);
	else if(pdevice->parity == PARITY_EVEN)
		strncpy(parity_str, "PARITY_EVEN", sizeof(parity_str) - 1);
	else if(pdevice->parity == PARITY_ODD)
		strncpy(parity_str, "PARITY_ODD ", sizeof(parity_str) - 1);
	printf("parity:    %s\n", parity_str);
}

void print_config(const UARTDevice* pdevice)
{
	if(pdevice == NULL)
	{
		printf("pdevice is NULL!");
		return;
	}

	printf("UART %s:\n", pdevice->pname);
	print_baud(pdevice);
	print_bits(pdevice);
	print_stop_bits(pdevice);
	print_parity(pdevice);
	printf("vmin:      %u bytes\n", pdevice->vmin);
	printf("vtime:     %.1f seconds\n", pdevice->vtime * 0.1);
}

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

	printf("Opening UART device %s\n", uart2.pname);
	if(uart_open(&uart2) != eUART_OK)
	{
		perror("Failed to open uart");
		exit(EXIT_FAILURE);
	}

	printf("Setting configuration...\n");
	print_config(&uart2);
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
	print_config(&uart2_check);

	printf("Comparing uart2 and uart2_check...\n");
	if(memcmp(&uart2, &uart2_check, sizeof(UARTDevice)) != 0)
	{
		perror("Comparison failed! uart2 and uart2_check are not the same.\n");
	}
	printf("Success! uart2 and uart2_check are the same\n");

	printf("testing loopback...\n");
	uint8_t send_buf[] = { 0x0A, 0x0B };
	uint8_t recv_buf[sizeof(send_buf)] = { 0 };
	printf("Sending: 0x%x, 0x%x\n", send_buf[0], send_buf[1]);
	size_t len = sizeof(send_buf);
	if(uart_write_all(&uart2, send_buf, &len) == eUART_WRITE_ERR)
	{
		perror("Failed to write");
		uart_close(&uart2);
		exit(EXIT_FAILURE);
	} 

	len = sizeof(recv_buf);
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
	printf("Received: 0x%x, 0x%x\n", recv_buf[0], recv_buf[1]);

	uart_close(&uart2);
	return 0;
}
