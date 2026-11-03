#define _POSIX_C_SOURCE 200809L

#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define DATA_SIZE (1024 * 1024 + 3)

/* Return 0 on success and -1 on failure.
 * Write tests create the file which are then readed by read tests
 * block_size is 1 or 4096 for block tests.
 */
 
int test_write(const char *path, const unsigned char *data, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)data;
    (void)size;
    (void)block_size;
    /* STAGE1: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  write block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_read(const char *path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)size;
    (void)block_size;
    /* STAGE1: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  read block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fwrite(const char *path, const unsigned char *data, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)data;
    (void)size;
    (void)block_size;
    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  fwrite block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fread(const char *path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)size;
    (void)block_size;
    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  fread block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fputc(const char *path, const unsigned char *data, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)data;
    (void)size;
    (void)block_size;
    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  fputc           %9.3f ms  OK\n", elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fgetc(const char *path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)size;
    (void)block_size;
    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  fgetc           %9.3f ms  OK\n", elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fscanf(const char *path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)path;
    (void)size;
    (void)block_size;
    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = -1;
    if (result == 0)
        printf("  fscanf           %9.3f ms  OK\n", elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

enum wait_mode
{
    WAIT_BUSY,
    WAIT_SLEEP,
    WAIT_POLL
};

/// Return 0 on success, -1 on error.
int test_wait(enum wait_mode mode)
{
    // initialization - boilerplate
    const char *name;
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd == -1)
    {
        perror("timerfd_create");
        return -1;
    }
    struct itimerspec timer = {.it_interval = {0, INTERVAL_NS}};
    if (clock_gettime(CLOCK_MONOTONIC, &timer.it_value) == -1)
    {
        perror("clock_gettime");
        close(fd);
        return -1;
    }
    timer.it_value.tv_nsec += INTERVAL_NS;
    if (timer.it_value.tv_nsec >= 1000000000L)
    {
        timer.it_value.tv_sec++;
        timer.it_value.tv_nsec -= 1000000000L;
    }
    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &timer, NULL) == -1)
    {
        perror("timerfd_settime");
        close(fd);
        return -1;
    }
    struct timer_stats stats = {.first_deadline = timer.it_value.tv_sec + timer.it_value.tv_nsec / 1e9};
    double wall = now(CLOCK_MONOTONIC);
    double cpu = now(CLOCK_PROCESS_CPUTIME_ID);

    int result = -1;

    // Real test starts here
    switch (mode)
    {
        case WAIT_BUSY:
        {
            name = "busy";
            // STAGE3: implement busy waiting here
            break;
        }
        case WAIT_SLEEP:
        {
            name = "sleep";
            // STAGE3: implement busy waiting with sleep here
            break;
        }
        case WAIT_POLL:
        {
            name = "poll";
            // STAGE3: implement waiting with poll here
            break;
        }
        default:
            return -1;
    }

    if (close(fd) == -1 || (result == 0 && stats.count != EXPIRATION_COUNT))
        result = -1;
    if (result == 0)
        printf(
            "  %s wall=%7.1f ms  CPU=%7.1f ms  expirations=%-2u  successful_reads=%-2u  avg delay=%6.2f ms  max "
            "delay=%6.2f ms  OK\n",
            name, elapsed(CLOCK_MONOTONIC, wall) * 1000, elapsed(CLOCK_PROCESS_CPUTIME_ID, cpu) * 1000, stats.count,
            stats.reads, stats.total_delay / stats.count * 1000, stats.max_delay * 1000);
    return result;
}

int main(void)
{
    static unsigned char data[DATA_SIZE];
    for (size_t i = 0; i < sizeof(data); i++)
        data[i] = (unsigned char)(i * 37 + i / 256);

    char directory[] = "/tmp/sop1-workshop-XXXXXX";
    if (!mkdtemp(directory))
    {
        perror("mkdtemp");
        return EXIT_FAILURE;
    }
    printf("Temporary directory: %s\n", directory);
    chdir(directory);

    int failed = 0;
    puts("POSIX: WRITE AND READ");
    if (report_result("write block=1", test_write("write_1", data, sizeof(data), 1)))
        failed = 1;
    if (report_result("read block=1", test_read("write_1", sizeof(data), 1)))
        failed = 1;
    if (report_result("write block=4096", test_write("write_4096", data, sizeof(data), 4096)))
        failed = 1;
    if (report_result("read block=4096", test_read("write_4096", sizeof(data), 4096)))
        failed = 1;

    puts("C STREAMS: FWRITE AND FREAD");
    if (report_result("fwrite block=1", test_fwrite("fwrite_1", data, sizeof(data), 1)))
        failed = 1;
    if (report_result("fread block=1", test_fread("fwrite_1", sizeof(data), 1)))
        failed = 1;
    if (report_result("fwrite block=4096", test_fwrite("fwrite_4096", data, sizeof(data), 4096)))
        failed = 1;
    if (report_result("fread block=4096", test_fread("fwrite_4096", sizeof(data), 4096)))
        failed = 1;

    puts("C STREAMS: CHARACTER AND FORMATTED I/O");
    if (report_result("fputc", test_fputc("fputc_path", data, sizeof(data), 0)))
        failed = 1;
    if (report_result("fgetc", test_fgetc("fputc_path", sizeof(data), 0)))
        failed = 1;
    if (report_result("fscanf", test_fscanf("fputc_path", sizeof(data), 0)))
        failed = 1;

    puts("WAITING FOR EVENTS");
    if (report_result("busy", test_wait(WAIT_BUSY)))
        failed = 1;
    if (report_result("sleep", test_wait(WAIT_SLEEP)))
        failed = 1;
    if (report_result("poll", test_wait(WAIT_POLL)))
        failed = 1;

    if (remove_test_file("write_1") == -1)
        failed = 1;
    if (remove_test_file("write_4096") == -1)
        failed = 1;
    if (remove_test_file("fwrite_1") == -1)
        failed = 1;
    if (remove_test_file("fwrite_4096") == -1)
        failed = 1;
    if (remove_test_file("fputc_path") == -1)
        failed = 1;
    if (rmdir(directory) == -1)
    {
        perror("rmdir");
        failed = 1;
    }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
