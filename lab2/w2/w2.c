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

// zapisac do pliku path dane z bufora data o wielkosci size w paczkach po block_size znakow
int test_write(const char* path, const unsigned char* data, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);

    /* STAGE1: implement the test, then set result to 0 on success. */
    int result = 0;

    int fd;
    // mkdtemp potem dba o to, ze ten folder musi byc nowy, wiec 0_EXCL jest w sumie useless
    if ((fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644)) == -1)
        return -1;

    size_t offset = 0;
    size_t how_much = 0;
    ssize_t read_bytes = 0;  // bo write moze zwrocic wartosc ujemna
    while (offset < size)
    {
        how_much = size - offset < block_size ? size - offset : block_size;
        // zapisuje do pliku fd zawartosc bufora data
        // offset we write w fd pamieta, gdzie skonczyl, ale date musimy pchac recznie
        read_bytes = write(fd, data + offset, how_much);

        if (read_bytes <= 0)
        {
            result = -1;  // bo musimy jeszcze zamknac plik
            break;
        }
        offset += read_bytes;
    }

    if (offset != size)
        result = -1;

    if (close(fd))  // chcemy to uwzglednic w pomiarze czasu
        result = -1;

    if (result == 0)
        printf("  write block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_read(const char* path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);

    /* STAGE1: implement the test, then set result to 0 on success. */
    int result = 0;

    if (block_size > 4096)  // zabezpieczenie
        return -1;
    char buf[4096];

    int fd;
    if ((fd = open(path, O_RDONLY)) == -1)
        return -1;

    size_t offset = 0;
    ssize_t read_bytes = 0;
    while (offset < size)
    {
        // za kazdym razem zapisujemy inna czesc danych z fd do tego samego miejsca w buf (nadpisujemy je ciagle)
        read_bytes = read(fd, buf, block_size);
        if (read_bytes < 0)
        {
            result = -1;
            break;
        }
        else if (read_bytes == 0)
            break;
        offset += read_bytes;
    }

    if (offset != size)
        result = -1;

    if (close(fd))
        result = -1;

    if (result == 0)
        printf("  read block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fwrite(const char* path, const unsigned char* data, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);

    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = 0;

    FILE* stream;
    // mkdtemp potem dba o to, ze ten folder musi byc nowy, wiec 0_EXCL jest w sumie useless
    if ((stream = fopen(path, "wx")) == NULL)  // x - exclusive
        return -1;

    size_t offset = 0;
    size_t how_much = 0;
    size_t read_bytes = 0;  // bo write moze zwrocic wartosc ujemna
    while (offset < size)
    {
        how_much = size - offset < block_size ? size - offset : block_size;
        // zapisuje do pliku fd zawartosc bufora data
        // offset we write w fd pamieta, gdzie skonczyl, ale date musimy pchac recznie
        read_bytes = fwrite(data + offset, 1, how_much, stream);

        if (read_bytes == 0)
        {
            result = -1;  // bo musimy jeszcze zamknac plik
            break;
        }
        offset += read_bytes;
    }

    if (offset != size)
        result = -1;

    if (fclose(stream))  // chcemy to uwzglednic w pomiarze czasu
        result = -1;

    if (result == 0)
        printf("  fwrite block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fread(const char* path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);

    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = 0;

    if (block_size > 4096)  // zabezpieczenie
        return -1;
    char buf[4096];

    FILE* stream;
    if ((stream = fopen(path, "r")) == NULL)
        return -1;

    size_t offset = 0;
    size_t read_bytes = 0;
    while (offset < size)
    {
        // za kazdym razem zapisujemy inna czesc danych z fd do tego samego miejsca w buf (nadpisujemy je ciagle)
        read_bytes = fread(buf, 1, block_size, stream);
        if (read_bytes == 0)  // jesli cos sie wywali, to ustawi 0
            break;
        offset += read_bytes;
    }

    if (offset != size)  // tutaj sprawdzamy czy doszlo do konca pliku czy nastapil blad
        result = -1;

    if (fclose(stream))
        result = -1;

    if (result == 0)
        printf("  fread block=%-4zu %9.3f ms  OK\n", block_size, elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

// zapisac do path dane z bufora data po 1 znaku size razy
int test_fputc(const char* path, const unsigned char* data, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)block_size;  // bo fputc zawsze czyta po 1 bajcie

    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = 0;

    FILE* stream;
    if ((stream = fopen(path, "wx")) == NULL)
        return -1;

    size_t offset = 0;
    int c = 0;
    while (offset < size)
    {
        c = fputc(*(data + offset), stream);  // oczekuje konkretnego znaku
        if (c == EOF)
            break;
        offset++;
    }

    if (offset != size)
        result = -1;

    if (fclose(stream))
        result = -1;

    if (result == 0)
        printf("  fputc           %9.3f ms  OK\n", elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fgetc(const char* path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)block_size;

    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = 0;

    FILE* stream;
    if ((stream = fopen(path, "r")) == NULL)
        return -1;

    size_t offset = 0;
    int c = 0;
    while (offset < size)
    {
        c = fgetc(stream);
        if (c == EOF)
            break;
        offset++;
    }

    if (offset != size)
        result = -1;

    if (fclose(stream))
        result = -1;

    if (result == 0)
        printf("  fgetc           %9.3f ms  OK\n", elapsed(CLOCK_MONOTONIC, start) * 1000);
    return result;
}

int test_fscanf(const char* path, size_t size, size_t block_size)
{
    double start = now(CLOCK_MONOTONIC);
    (void)block_size;

    /* STAGE2: implement the test, then set result to 0 on success. */
    int result = 0;

    FILE* stream;
    if ((stream = fopen(path, "r")) == NULL)
        return -1;

    size_t offset = 0;
    char c;  // bufor na znak
    int ret = 0;
    while (offset < size)
    {
        ret = fscanf(stream, "%c", &c);  // wczytuje ze stream znak do zmiennej c i modyfikuje oryginal
        if (ret != 1)                    // nie wczytano 1 znaku, tak jak mialo byc
            break;
        offset++;
    }

    if (offset < size)
        result = -1;

    if (fclose(stream))
        result = -1;

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
    const char* name;
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
            result = 0;

            while (stats.count < EXPIRATION_COUNT)  // az zbierzemy wymagana liczbe tykniec timera
            {
                uint64_t expirations;
                ssize_t n = read(fd, &expirations, sizeof(expirations));
                // chcemy przeczytac 8 bajtow z deskryptora timera

                if (n == (ssize_t)sizeof(expirations))  // timer zadzwonil
                    record_expirations(&stats, expirations, now(CLOCK_MONOTONIC));
                else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))  // jeszcze nie ma danych
                    continue;
                else  // po prostu blad
                {
                    result = -1;
                    break;
                }
            }

            break;
        }
        case WAIT_SLEEP:
        {
            name = "sleep";
            // STAGE3: implement busy waiting with sleep here
            result = 0;

            while (stats.count < EXPIRATION_COUNT)
            {
                uint64_t expirations;
                ssize_t n = read(fd, &expirations, sizeof(expirations));

                if (n == (ssize_t)sizeof(expirations))
                {
                    record_expirations(&stats, expirations, now(CLOCK_MONOTONIC));
                }
                else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    // Zamiast od razu pytać znowu, idziemy spać na 100 mikrosekund
                    struct timespec req = {0, 100000};
                    nanosleep(&req, NULL);
                    continue;
                }
                else
                {
                    result = -1;
                    break;
                }
            }

            break;
        }
        case WAIT_POLL:
        {
            name = "poll";
            // STAGE3: implement waiting with poll here
            result = 0;

            struct pollfd pfd;
            pfd.fd = fd;          // Obserwujemy nasz deskryptor od timera
            pfd.events = POLLIN;  // Interesuje nas tylko moment, gdy pojawią się dane do odczytu

            // Czekamy, aż zbierzemy odpowiednią liczbę tyknięć timera
            while (stats.count < EXPIRATION_COUNT)
            {
                // Usypiamy proces za pomocą poll()
                // Argumenty: (tablica z obserwowanymi strukturami, ilość elementów w tablicy, timeout w ms)
                int ret = poll(&pfd, 1, -1);  // musi byc adres, bo musi byc wskaznik

                // Sprawdzamy, czy poll() obudziło się z błędem
                if (ret == -1)
                {
                    result = -1;
                    break;
                }

                // Skoro poll() nas obudziło bez błędu, to znaczy, że timer tyknął i zdarzenie POLLIN nadeszło
                // Teraz mamy gwarancję, że read NIE ZWRÓCI EAGAIN
                uint64_t expirations;

                // Odczytujemy dane
                ssize_t n = read(fd, &expirations, sizeof(expirations));

                // Weryfikacja odczytu
                if (n == (ssize_t)sizeof(expirations))
                {
                    // Sukces - odnotowujemy tyknięcia w strukturze statystyk
                    record_expirations(&stats, expirations, now(CLOCK_MONOTONIC));
                }
                else
                {
                    // Jeśli read nie wczytało 8 bajtów (rozmiar uint64_t) lub wystąpił błąd, przerywamy
                    // Nie sprawdzamy tu już EAGAIN, bo użycie poll() gwarantuje nam, że dane
                    // na pewno tam były przed odczytem
                    result = -1;
                    break;
                }
            }

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
