#include <gpiod.h>  // requires to add the flag '-lgpiod' when compiling
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define CHIPNAME "/dev/gpiochip0"
#define LINE_NUM 17 // GPIO pin number

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) // function that handles SIGINT interrupt (happens when we press `ctrl` + `c` to stop the process)
{
    stop = 1;
}

int main(void)
{
    signal(SIGINT, handle_sigint);
    struct timespec ts = {0, 500000000};

    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret_val;

    // open GPIO chip
    chip = gpiod_chip_open(CHIPNAME);
    if(chip == NULL)
    {
        perror("Open chip failed\n");
        return 1;
    }

    // get GPIO line
    line = gpiod_chip_get_line(chip, LINE_NUM);
    if(line == NULL)
    {
        perror("Get line failed\n");
        gpiod_chip_close(chip);
        return 1;
    }

    // request line as output, default OFF
    ret_val = gpiod_line_request_output(line, "blink", 0);  // "blink" is just a name we choose to identify the service that uses that pin
    if(ret_val < 0)
    {
        perror("Request line as output failed\n");
        gpiod_chip_close(chip);
        return 1;
    }

    while(!stop)    // or "for(int i = 0; i < 5; ++i)"
    {
        gpiod_line_set_value(line, 1);  // sends `1` to `line` - LED ON
        nanosleep(&ts, NULL);
        gpiod_line_set_value(line, 0);  // sends `0` to `line` - LED OFF
        nanosleep(&ts, NULL);
    }

    // release line and close chip
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return 0;
}