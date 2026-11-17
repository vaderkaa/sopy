#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define MAX_SMBOX_MSGS 25
#define BATCH_BYTES 64
#define MAX_FIFOS 4

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef struct
{
    char h_size;
    char m_size;
} Header;

static void usage(char* name)
{
    fprintf(stderr, "USAGE: %s mailbox_file [fifo ...]\n", name);
    exit(EXIT_FAILURE);
}

static inline void msleep(int ms)
{
    struct timespec time = {.tv_sec = 0, .tv_nsec = 1000000 * ms};
    nanosleep(&time, NULL);
}

int main(int argc, char** argv)
{
    if (argc < 2)
        usage(argv[0]);

    int fd;
    if ((fd = open(argv[1], O_RDONLY)) == -1)
        ERR("open");

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1)
        ERR("fstat");
    size_t file_size = file_stat.st_size;

    // char* file_mmap = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    // if (file_mmap == MAP_FAILED)
    //     ERR("mmap");

    // int position = 0;
    // char* h_size;
    // char* m_size;
    // char* header;
    // char* message;
    // while (position < file_size)
    // {
    //     if (sscanf(file_mmap, "%c[^0]%c[^0}%s[^0]%s[^0]", h_size, m_size, header, message) != 4)
    //     {
    //         fprintf(stderr, "Zly format\n");
    //         if (close(fd))
    //             ERR("close");
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // if (munmap(file_mmap, file_size) == -1)
    //     ERR("munmap");

    // struct pollfd fds[1];

    // Wczytujemy w trybie nieblokujacym
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
        ERR("fcntl get");
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1)
        ERR("fcntl set");

    // fds[0].fd = STDIN_FILENO;
    // fds[0].events = POLLIN;
    // fds[0].revents = 0;

    int message_number = 0;

    // int active_fds = 1;
    // while (active_fds > 0)
    ssize_t bytes_read = 0;
    char buf[BATCH_BYTES + 1];
    ssize_t all = 0;

    while (all < BATCH_BYTES)
    {
        bytes_read = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (bytes_read > 0)  // Sa dane
        {
            all += bytes_read;
            buf[bytes_read] = '\0';
            for (size_t k = 0; k < strlen(buf); k++)
            {
                if (buf[k] == 'w')  // Poprzednia
                {
                    if (message_number == 0)
                        fprintf(stdout, "Selected message: %d\n", message_number);
                    else
                    {
                        message_number--;
                        fprintf(stdout, "Selected message: %d\n", message_number);
                    }
                    fprintf(stdout, "Printing headers...\n");
                }
                else if (buf[k] == 's')  // Nastepna
                {
                    message_number++;
                    fprintf(stdout, "Selected message: %d\n", message_number);
                    fprintf(stdout, "Printing headers...\n");
                }
                else if (buf[k] == 'd')
                {
                    fprintf(stdout, "Dumping file...\n");

                    char* file_mmap = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
                    if (file_mmap == MAP_FAILED)
                        ERR("mmap");

                    fflush(stdout);

                    ssize_t write_bytes = write(STDIN_FILENO, file_mmap, file_size);
                    if (write_bytes < 0)
                        ERR("write");

                    if (munmap(file_mmap, file_size) == -1)
                        ERR("munmap");
                }
                else if (buf[k] == 'h')
                {
                    fprintf(stdout, "Printing headers...\n");
                }
                else if (buf[k] == 'm')
                {
                    fprintf(stdout, "Printing message...\n");
                }
                else if (buf[k] == 'q')  // Zwolnienie zasobow i zamkniecie aplikacji
                {
                    if (close(fd))
                        ERR("close");
                    all = BATCH_BYTES;
                }
                else if (buf[k] != '\n')
                    printf("Invalid input\n");
            }
        }
        else if (bytes_read == 0)  // EOF
        {
            if (close(fd))
                ERR("close");
            break;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)  // Nic sie nie pojawilo
        {
            msleep(100);
            continue;
        }
        else
        {
            ERR("read");
        }
    }
}
