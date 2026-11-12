#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define EXPIRATION_COUNT 20
#define INTERVAL_NS 30000000L

double now(clockid_t clock_id)
{
    struct timespec time;
    if (clock_gettime(clock_id, &time) == -1)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)time.tv_sec + (double)time.tv_nsec / 1000000000.0;
}

double elapsed(clockid_t clock_id, double start) { return now(clock_id) - start; }

struct timer_stats
{
    unsigned count;
    unsigned reads;
    double first_deadline;
    double total_delay;
    double max_delay;
};

// Record when each scheduled expiration was observed. 
void record_expirations(struct timer_stats *stats, uint64_t expirations, double received)
{
    stats->reads++;
    while (expirations > 0 && stats->count < EXPIRATION_COUNT)
    {
        double deadline = stats->first_deadline + stats->count * (INTERVAL_NS / 1e9);
        double delay = received - deadline;
        stats->total_delay += delay;
        if (delay > stats->max_delay)
            stats->max_delay = delay;
        stats->count++;
        expirations--;
    }
}

static int report_result(const char *name, int result)
{
    if (result != 0) {
        printf("  %s FAIL\n", name);
        return 1;
    }
    return 0;
}

static int remove_test_file(const char *path)
{
    if (unlink(path) == -1 && errno != ENOENT) {
        perror("unlink");
        return -1;
    }
    return 0;
}