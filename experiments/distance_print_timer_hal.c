#include "../src/hal/uart/hal_uart.h"

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
#include "../src/hal/uart/hal_uart_config.h"


int main(void)
{
    int uart_fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);
    if(uart_fd < 0)
    {
        perror("Mey");
        return 1;
    }

    if(!isatty(uart_fd))
	{
		close(uart_fd);
		return 1;
	}

    struct termios tty = {
		.c_iflag = 0,                       // Turn off all input processing
		.c_oflag = 0,                       // Turn off all output processing
		.c_cflag = (CLOCAL | CREAD | CS8),  // Ignore modem and enable reads
		.c_lflag = 0,                       // Turn off all local flags
        .c_cc = { 0 }                       // Ignore all special characters
	};

    tty.c_cc[VMIN]  = 0;  // Minimal number of read bytes
	tty.c_cc[VTIME] = 0; // Timeout in 0.1 second units

    if(cfsetispeed(&tty, B9600) == -1 ||
	   cfsetospeed(&tty, B9600) == -1)
	{
        close(uart_fd);
		return 1;
	}

    if(tcsetattr(uart_fd, TCSAFLUSH, &tty) == -1)
	{
        close(uart_fd);	// here we put close because Dani doesn't like 'em
		return 1;
	}

	int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if(timer_fd == -1)
	{
		perror("very sad");
		// here we didn't put close because Dani hit me
		return 1;
	}

	struct itimerspec its = {
    	.it_interval 	= { .tv_sec = 3, .tv_nsec = 0 },
		.it_value 		= { .tv_sec = 3, .tv_nsec = 0 }
	};

	if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
        perror("very sad");
        return 1;
    }

	struct pollfd fds[2] = {
		[0] = { .fd = uart_fd, .events = POLLIN },
		[1] = { .fd = timer_fd, .events = POLLIN }
	};

    const uint8_t command[] = { 0x57, 0x10, 0xff, 0xff, 0x00, 0xff, 0xff, 0x63 };
	uint8_t read_buff[16];
	struct timespec ts = {1, 0};
    while(1)
	{
		write(uart_fd, command, sizeof(command));

		int ret = poll(fds, 2, -1);  // block indefinitely
        if (ret == -1)
		{
            if (errno == EINTR || errno == EAGAIN) continue;
            perror("poll");
            break;
        }

		if (fds[0].revents & POLLIN)
		{
			if (timerfd_settime(timer_fd, 0, &its, NULL) == -1)
			{
				perror("very sad");
        		return 1;
			}
			read(uart_fd, read_buff, sizeof(read_buff));
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
		
		if (fds[1].revents & POLLIN)
		{
			uint64_t expirations;
			if (read(timer_fd, &expirations, sizeof(expirations)) != sizeof(expirations))
			{
				perror("read timerfd");
				break;
			}
			printf("\nTimer expired!\n");
        }
	}

    return 0;
}