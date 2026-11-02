#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int main(void)
{
    struct pollfd fds[1]; // Potrzebujemy tablicy tylko na jeden deskryptor
    char buf[256];        // Bufor tekstowy zamiast struktury input_event

    // Wcześniej flaga O_NONBLOCK była podawana w funkcji open().
    // Ponieważ stdin (deskryptor 0) jest już otwarty przez system dla Twojego terminala,
    // używamy funkcji fcntl, aby "dokleić" mu właściwość nieblokującą.
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
        ERR("fcntl get");
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1)
        ERR("fcntl set");

    // Konfiguracja struktury dla poll
    fds[0].fd = STDIN_FILENO; // STDIN_FILENO to po prostu bezpieczne makro na cyfrę 0
    fds[0].events = POLLIN;   // Chcemy wiedzieć, kiedy pojawi się tekst
    fds[0].revents = 0;       // Zerujemy śmieci w pamięci

    printf("Wpisz tekst i wcisnij Enter (wcisnij Ctrl+D aby zakonczyc).\n");

    for (int j = 0; j < 30; j++) // Poll ma zwrocic 30 razy
    {
        // Program zasypia (timeout -1) i czeka na wciśnięcie klawisza Enter (bo tak dziala terminal)
        int ret = poll(fds, 1, -1);
        if (ret < 0)
            ERR("poll");

        // Jeśli strumień został przerwany (np. przez Ctrl + D, czyli EOF)
        if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            printf("\nStrumien zamkniety (POLLHUP). Koniec programu.\n");
            break;
        }

        // Jeśli są nowe znaki do odczytu
        if (fds[0].revents & POLLIN)
        {
            ssize_t r;
            // Pętla wysysa z bufora wszystko, co wpisałaś, zanim enter ją obudził
            while ((r = read(fds[0].fd, buf, sizeof(buf) - 1)) > 0)
            {
                buf[r] = '\0'; // Dodajemy null-terminator, by bezpiecznie wypisać tekst
                printf("=> Odczytano %zd bajtow: %s", r, buf);
            }

            // Jeśli read zwróci 0, oznacza to "End of File" (Ctrl + D)
            if (r == 0)
            {
                printf("\nKoniec wejscia (EOF). Koniec programu.\n");
                break;
            }
        }
    }

    return 0;
}