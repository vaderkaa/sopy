#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
#include <sys/mman.h>

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
            fprintf(stderr, "Ta księga to nie przepis!\n");
            exit(EXIT_FAILURE);
        }
    }
    else // Po prostu blad open
        ERR("open");

    // PRZYGOTOWANIE STRUKTURY POLLFD
    int fifo;
    struct stat file_stat;
    struct pollfd pfd[argc - 2]; // Minus nazwa programu i przepis
    for (int i = 2; i < argc; i++)
    {
        if ((fifo = open(argv[i], O_RDONLY | O_NONBLOCK)) == -1) // TUTAJ MUSI BYC NONBLOCK
        {
            if (errno == ENOENT) // Plik nie istnieje
            {
                fprintf(stderr, "Nie ma babci na wieży %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            else
                ERR("open");
        }

        if (fstat(fifo, &file_stat) == -1)
            ERR("fstat");
        if (!S_ISFIFO(file_stat.st_mode)) // Plik nie jest laczem nazwanym
        {
            fprintf(stderr, "Nie ma babci na wieży %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }

        pfd[i - 2].fd = fifo;
        pfd[i - 2].events = POLLIN;
        pfd[i - 2].revents = 0;
    }

    int ret = 0;
    ssize_t bytes_read = 0;

    char main_buf[argc - 2][4096];         // Kazda babcia musi miec wlasny bufor
    memset(main_buf, 0, sizeof(main_buf)); // Zeby nie bylo smieci tylko czysto
    char buf[128] = "";
    char *pointer = NULL;
    int number = 0;
    char line_temp[64] = "";
    char line[64] = "";

    int closed_files = 0;

    while (closed_files < (argc - 2))
    {
        ret = poll(pfd, argc - 2, 3000); // TUTAJ TRZEBA ZMIENIC DRUGI PARAMETR
        // 3000 - 3 sekundy

        if (ret == -1) // Czy mamy blad polla po prostu
            ERR("poll");

        if (ret == 0) // Minely 3 sekundy i nic sie nie wydarzylo
        {
            fprintf(stdout, "Babcie dolewają herbatki…\n");
            continue;
        }

        for (int i = 0; i < argc - 2; i++) // Idziemy po kolei po kazdym fifo
        {
            if (pfd[i].fd < 0) // Zamknelismy juz ten fifo
                continue;

            if (pfd[i].revents & (POLLERR | POLLNVAL)) // Czy w bitach zwrotnych znajduje sie jakakolwiek z tych flag
            {
                if (close(pfd[i].fd))
                    ERR("close");
                closed_files++;
                pfd[i].fd = -1; // Ignorujemy zepsute urzadzenie w nastepnych obrotach petli
                continue;
            }

            // Jesli na urzadzeniu nie ma nowych danych ani sie nie rozlaczylo
            if (!(pfd[i].revents & (POLLIN | POLLHUP)))
                continue;
            // Jesli babcia dala dane i w tej samej sekundzie sie rozlaczyla, to cos moglo zostac w srodku

            while ((bytes_read = read(pfd[i].fd, buf, sizeof(buf) - 1)) > 0)
            {
                // ETAP 1
                // buf[bytes_read] = '\0';
                // fprintf(stdout, "%s", buf);

                buf[bytes_read] = '\0';
                strcat(main_buf[i], buf);

                while ((pointer = strchr(main_buf[i], '\n')) != NULL)
                {
                    *pointer = '\0';

                    // Szuka w main_buf maksmalnie 2 liczby, potem ma byc dwukropek, a potem cokolwiek, az do entera
                    // Ale tego entera na pewno nie ma, wiec przeczyta calosc, az do znaku \0
                    // Nie moge napisac zwyklego %s, bo wtedy przerwie mi po pierwszej spacji
                    // Dokleja na koniec \0
                    if (sscanf(main_buf[i], "%2d:%63[^\n]", &number, line_temp) == 2 && number >= 0 && number <= 31)
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
                        fprintf(stderr, "Babcia %s się zagadała\n", argv[i + 2]);

                    // Przesuwamy to, co jest za pointerem, razem ze znakiem konca linii
                    memmove(main_buf[i], pointer + 1, strlen(pointer + 1) + 1);
                }
            }
            if (bytes_read == 0) // EOF
            {
                fprintf(stdout, "Babcia z wieży %s skończyła dyktować\n", argv[i + 2]);
                if (close(pfd[i].fd))
                    ERR("close");
                closed_files++;
                pfd[i].fd = -1;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) // Nie ma juz danych, ale jeszcze nie zamknieto fifo
                continue;
            else // Po prostu blad
                ERR("read");
        }
    }

    // MAPOWANIE
    char *mmap_buf = mmap(NULL, 2048, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_buf == MAP_FAILED)
        ERR("mmap");

    // ZAMIANA raw~berries na raspberries
    *(mmap_buf + 2047) = '\0';
    char *mmap_pointer = mmap_buf;
    int count = 0;
    while ((mmap_pointer = strstr(mmap_pointer, "raw~berries")) != NULL)
    {
        memcpy(mmap_pointer, "raspberries", sizeof("raspberries") - 1);
        count++;
        mmap_pointer += sizeof("raspberries") - 1;
    }
    *(mmap_buf + 2047) = '\n';
    msync(mmap_buf, 2048, MS_SYNC);
    fprintf(stdout, "Brudy księżniczki Joanny wyczyszczone: %d\n", count);

    // LICZENIE BRAKUJACYCH LINII I WYSWIETLANIE PRZEPISU
    char mmap_line[65];
    mmap_line[64] = '\0'; // Zabezpieczenie
    int j = 0;
    int missing = 0;
    char ready[32][64];
    for (int i = 0; i < 32; i++)
    {
        memcpy(mmap_line, mmap_buf + i * 64, 64);

        if (strspn(mmap_line, " ") == 63)
        {
            missing++;
            ready[i][0] = '\0';
            continue;
        }

        j = 62; // Startuje przed znakiem entera
        while (j >= 0 && mmap_line[j] == ' ')
            j--;
        mmap_line[j + 1] = '\0';

        strcpy(ready[i], mmap_line);
    }

    for (int i = 0; i < 32; i++)
    {
        if (ready[i][0] != '\0')
            fprintf(stdout, "%02d: %s\n", i, ready[i]);
    }

    if (missing == 0)
        fprintf(stdout, "Tort gotowy!\n");
    else
        fprintf(stdout, "Brakuje %d kroków, tort się zapadnie!\n", missing);

    munmap(mmap_buf, 2048);

    if (close(fd))
        ERR("close");

    return EXIT_SUCCESS;
}