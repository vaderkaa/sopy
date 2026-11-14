// Skanowanie katalogu (opendir/readdir)
// Otwieranie eventów z O_NONBLOCK i multipleksowanie wielu fd przez poll()
// Kompilacja: gcc -o t4_poll t4_poll.c

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_DEVS 32

int is_event_name(const char *name) // Wybiera pliki o nazwach event1234567890
{
    if (strncmp(name, "event", 5) != 0) // String wejsciowy, wzorzec, liczba znakow do sprawdzenia, zwraca 0, jesli sa identyczne
        return 0;
    for (int i = 5; name[i] != '\0'; i++)
    {
        if (!isdigit(name[i])) // Jesli znak NIE jest cyfra
            return 0;
    }
    return 1;
}

int get_descriptors(struct pollfd fds[MAX_DEVS], char paths[MAX_DEVS][512]) // Zwraca ostateczna liczbe otwartych plikow
{
    int n = 0;
    DIR *dir = opendir("/dev/input");
    if (!dir)
    {
        ERR("opendir");
    }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL)
    {
        if (!is_event_name(de->d_name)) // Jesli nazwa pliku nie jest obslugiwanym przez nas zdarzeniem event123
            continue;
        if (n >= MAX_DEVS) // Mamy juz 32 urzadzenia, nie ma miejsca na kolejne
            break;

        snprintf(paths[n], sizeof(paths[n]), "/dev/input/%s", de->d_name);
        // Gdzie wpiszemy tekst, ile maksymalnie znakow, jaki tekst wpiszemy

        int fd = open(paths[n], O_RDONLY | O_NONBLOCK); // Tylko do odczytu, dzialanie nieblokujace
        if (fd < 0)                                     // Nie mozna otworzyc pliku (np. przez brak uprawnien)
            ERR("open");

        fds[n].fd = fd;
        fds[n].events = POLLIN; // Gotowosc do odczytania nowych danych
        fds[n].revents = 0;     // Wyzerowanie pola zwracanych zdarzen
        n++;
    }
    closedir(dir);
    return n;
}

int main(void)
{
    struct pollfd fds[MAX_DEVS];         // Miejsce na 32 urzedzenia
    char paths[MAX_DEVS][512];           // 32 wiersze po 512 znakow - sciezki do plikow
    int n = get_descriptors(fds, paths); // Pobieramy liczbe urzadzen, tworzymy sobie tablice file descriptors i sciezek do nich

    if (n == 0) // Nie otwarto zadnego urzadzenia
    {
        fprintf(stderr, "No readable event devices (check permissions)\n");
        return 1;
    }

    for (int j = 0; j < 30; j++)
    {
        int ret = poll(fds, n, -1); // Ile struktur zarejestrowalo zdarzenia (pole revents rozne od 0)
        if (ret < 0)                // Blad
            ERR("poll");

        for (int i = 0; i < n; i++)
        {
            if (fds[i].fd < 0) // Ignorujemy martwy, odlaczony sprzet
                continue;

            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) // Czy w bitach zwrotnych znajduje sie jakakolwiek z tych flag
            {
                close(fds[i].fd);
                fds[i].fd = -1; // Ignorujemy zepsute urzadzenie w nastepnych obrotach petli
                continue;
            }

            if (!(fds[i].revents & POLLIN)) // Na urzadzeniu NIE ma nowych danych
                continue;

            struct input_event ev;
            ssize_t r;
            while ((r = read(fds[i].fd, &ev, sizeof(ev))) == sizeof(ev)) // Wkleja do ev 24 bajty
            {
                printf("%s: %ld.%06ld type=%u code=%u value=%d\n", paths[i], (long)ev.input_event_sec,
                       (long)ev.input_event_usec, ev.type, ev.code, ev.value);
            }
        }
    }

    for (int i = 0; i < n; i++)
        if (fds[i].fd >= 0)
            close(fds[i].fd);

    return 0;
}