#include "hal_uart.h"

/* Standard Libraries */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>

/* Linux Specific Libraries */
#include <termios.h>

/* User Libraries */
#include "hal_uart_config.h"


int main(void)
{
    int fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);
    if(fd < 0)
    {
        perror("Mey");
        return 1;
    }

    if(!isatty(fd))
	{
		close(fd);
		return 1;
	}

    struct termios tty = {
		.c_iflag = 0,                       // Turn off all input processing
		.c_oflag = 0,                       // Turn off all output processing
		.c_cflag = (CLOCAL | CREAD | CS8),  // Ignore modem and enable reads
		.c_lflag = 0,                       // Turn off all local flags
        .c_cc = { 0 }                       // Ignore all special characters
	};

    tty.c_cc[VMIN]  = 1;  // Minimal number of read bytes
	tty.c_cc[VTIME] = 1; // Timeout in 0.1 second units

    if(cfsetispeed(&tty, B9600) == -1 ||
	   cfsetospeed(&tty, B9600) == -1)
	{
        close(fd);
		return 1;
	}

    if(tcsetattr(fd, TCSAFLUSH, &tty) == -1)
	{
        close(fd);
		return 1;
	}

    const uint8_t command[] = { 0x57, 0x10, 0xff, 0xff, 0x00, 0xff, 0xff, 0x63 };
	uint8_t read_buff[16];
	struct timespec ts = {1, 0};
    while(1)
	{
		write(fd, command, sizeof(command));
		read(fd, read_buff, sizeof(read_buff));
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
	}

    return 0;
}