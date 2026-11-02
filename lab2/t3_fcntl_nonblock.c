// Dodawanie O_NONBLOCK do otwartego pliku (fcntl)
// Pętla read z obsługą EAGAIN / EWOULDBLOCK i EOF
// Kompilacja: gcc -o t3_fcntl_nonblock t3_fcntl_nonblock.c

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL); // Pobiera wszystkie obecne ustawienia tego pliku
    if (flags == -1)
        ERR("fcntl");
    flags |= O_NONBLOCK;                 // Dodaje flage do ustawien
    if (fcntl(fd, F_SETFL, flags) == -1) // SETFL - set file status flags
        ERR("fcntl");
}

int main(void)
{
    char buf[128];
    ssize_t n;

    puts("Testing blocking scanf - type something and press enter:");
    scanf("%100s", buf); // Wczyta maksymalnie 100 znakow
    printf("You typed: %s\n", buf);

    puts("Testing blocking read - type something and press enter:");
    n = read(STDIN_FILENO, buf, sizeof(buf) - 1); // STDIN_FILENO - stala 0, oznaczajaca standardowe wejscie, maksymalnie 127 znakow
    if (n < 0)
        ERR("read");
    buf[n] = 0;                                // Dodaje znak konca stringa \0
    printf("Got %lu bytes: \"%s\"\n", n, buf); // long unsigned, do ssize_t mozna tez zd
    if (buf[n - 1] == '\n')                    // Sprawdza czy ostatni wpisany przez uzytkownika znak to enter
        puts("(your last character was \\n)"); // Traktuje \n jako dwa znaki normalne

    set_nonblocking(STDIN_FILENO);
    printf("Nonblocking read for ~10s, type something and press enter\n");

    for (int i = 0; i < 10; i++)
    {
        printf("Try %d/10...\n", i + 1);
        n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0) // Zdazylam cos wpisac
        {
            buf[n] = '\0';
            printf("Got %zd bytes: %s", n, buf);
        }
        else if (n == 0)
        {
            printf("EOF\n"); // End of file (kiedy zrobie Ctrl + D i zamkne strumien)
            break;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("No data yet (EAGAIN)...\n"); // Nie zdazylam nic wpisac
        }
        else
        {
            ERR("read"); // Prawdziwy blad funkcji
        }
        sleep(1);
    }
    return 0;
}