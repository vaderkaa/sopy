#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE: %s\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int fd;
    char *recipe = argv[1];
    if ((fd = open(recipe, O_CREAT | O_EXCL | O_RDWR, 0666)) != -1) // Nie istnieje
    {
        char line[64];
        for (int i = 0; i < 32; i++)
        {
            memset(line, ' ', 63);
            line[63] = '\n';
            if (write(fd, line, 64) == -1)
                ERR("write");
        }
    }
    else if (errno == EEXIST) // Istnieje
    {
        if ((fd = open(recipe, O_RDWR)) == -1)
            ERR("open");
        if (lseek(fd, 0, SEEK_END) != 2048)
        {
            fprintf(stderr, "Ta ksiega to nie przepis!\n");
            exit(EXIT_FAILURE);
        }
    }
    else // Po prostu blad open
        ERR("open");

    int fifo;
    if ((fifo = open(argv[2], O_RDONLY)) == -1)
    {
        if (errno == ENOENT) // Plik nie istnieje
        {
            fprintf(stderr, "Nie ma babci na wieży %s\n", argv[2]);
            exit(EXIT_FAILURE);
        }
        else
            ERR("open");
    }

    struct stat file_stat;
    if (fstat(fifo, &file_stat) == -1)
        ERR("fstat");
    if (!S_ISFIFO(file_stat.st_mode)) // Plik nie jest laczem nazwanym
    {
        fprintf(stderr, "Nie ma babci na wieży %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    struct pollfd pfd;
    pfd.fd = fifo;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int ret = 0;
    ssize_t bytes_read = 0;

    char main_buf[4096] = "";
    char buf[128] = "";
    char *pointer = NULL;
    int number = 0;
    char line_temp[64] = "";
    char line[64] = "";

    for (;;)
    {
        ret = poll(&pfd, 1, -1);

        if (ret == -1) // Czy mamy blad polla po prostu
            ERR("poll");

        while ((bytes_read = read(fifo, buf, sizeof(buf) - 1)) > 0)
        {
            // ETAP 1
            // buf[bytes_read] = '\0';
            // fprintf(stdout, "%s", buf);
            buf[bytes_read] = '\0';
            strcat(main_buf, buf);

            while ((pointer = strchr(main_buf, '\n')) != NULL)
            {
                *pointer = '\0';

                // Szuka w main_buf maksmalnie 2 liczby, potem ma byc dwukropek, a potem cokolwiek, az do entera
                // Ale tego entera na pewno nie ma, wiec przeczyta calosc, az do znaku \0
                // Nie moge napisac zwyklego %s, bo wtedy przerwie mi po pierwszej spacji
                // Dokleja na koniec \0
                if (sscanf(main_buf, "%2d:%63[^\n]", &number, line_temp) == 2 && number >= 0 && number <= 31)
                {
                    memset(line, ' ', 63);
                    line[63] = '\n';
                    memcpy(line, line_temp, strlen(line_temp) > 63 ? 63 : strlen(line_temp));

                    lseek(fd, number * 64, SEEK_SET);
                    if (write(fd, line, 64) == -1)
                        ERR("write");

                    fprintf(stdout, "Krok %02d zapisany\n", number);
                }
                else
                    fprintf(stderr, "Babcia sie zagadala\n");

                // Przesuwamy to, co jest za pointerem, razem ze znakiem konca linii
                memmove(main_buf, pointer + 1, strlen(pointer + 1) + 1);
            }
        }
        if (bytes_read == 0) // EOF
            break;
        else if (errno == EAGAIN || errno == EWOULDBLOCK) // Nie ma juz danych, ale jeszcze nie zamknieto fifo
            continue;
        else // Po prostu blad
            ERR("read");
    }

    if (close(fifo))
        ERR("close");

    if (close(fd))
        ERR("close");
}