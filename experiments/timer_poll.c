// instead of defining _GNU_SOURCE we compile with -D_GNU_SOURCE flag
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/poll.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main(void) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (tfd == -1) {
        perror("timerfd_create");
        exit(1);
    }

    // Set timer to expire in 5 seconds (absolute time).
    struct itimerspec its;
    memset(&its, 0, sizeof(its));

    its.it_value.tv_sec = 5; // absolute timeout
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 5;
    its.it_interval.tv_nsec = 0;

    if (timerfd_settime(tfd, 0, &its, NULL) == -1) {
        perror("timerfd_settime");
        exit(1);
    }

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = tfd;
    fds[1].events = POLLIN;

    for (;;) {
        printf("\nWaiting for input on stdin or timer (5s)...\n");

        int ret = poll(fds, 2, -1);  // block indefinitely
        if (ret == -1) {
            if (errno == EINTR || errno == EAGAIN) continue;
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            char buf[128];
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("\nGot input: %s", buf);
            }
        }

        if (fds[1].revents & POLLIN) {
            uint64_t expirations;
            if (read(tfd, &expirations, sizeof(expirations)) != sizeof(expirations)) {
                perror("read timerfd");
                break;
            }
            printf("\nTimer expired!\n");
        }
    }

    close(tfd);
    return 0;
}