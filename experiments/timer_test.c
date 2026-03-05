#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define TARGET_DURATION_NS 1000000  // 1 millisecond = 1,000,000 nanoseconds
#define TIMER_INTERVAL_NS  100000    // 100 microseconds = 100,000 nanoseconds


int main()
{
    struct timespec res;

    clock_getres(CLOCK_MONOTONIC, &res);
    printf("CLOCK_MONOTONIC resolution: %ld ns\n", res.tv_nsec);

    clock_getres(CLOCK_REALTIME, &res);
    printf("CLOCK_REALTIME resolution: %ld ns\n", res.tv_nsec);

    clock_getres(CLOCK_MONOTONIC_RAW, &res);
    printf("CLOCK_MONOTONIC_RAW resolution: %ld ns\n", res.tv_nsec);

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1) {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }

    struct timespec start, now;
    uint64_t expirations;
    int count = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        // Set single-shot timer for 10 µs
        struct itimerspec new_timer = {
            .it_interval = {0, 0},  // No repeat
            .it_value = {0, TIMER_INTERVAL_NS}
        };

        if (timerfd_settime(tfd, 0, &new_timer, NULL) == -1) {
            perror("timerfd_settime");
            break;
        }

        // Wait for timer to expire
        if (read(tfd, &expirations, sizeof(expirations)) != sizeof(expirations)) {
            perror("read");
            break;
        }

        count++;

        // Check elapsed time
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t elapsed_ns = (now.tv_sec - start.tv_sec) * 1000000000ULL +
                              (now.tv_nsec - start.tv_nsec);
        if (elapsed_ns >= TARGET_DURATION_NS) {
            break;
        }
    }

    close(tfd);
    printf("Timer fired %d times in 1 ms\n", count);
    return EXIT_SUCCESS;
}