#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define MAX_FDS 2 // Liczba deskryptorow, ktorych nasluchujemy

int main() {
    // =================================================================
    // 1. TWORZENIE I INICJALIZACJA STRUKTURY FDS
    // =================================================================
    struct pollfd fds[MAX_FDS];

    // Przyklad A: nasluchujemy na standardowym wejsciu (klawiatura, fd = 0)
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN; // Chcemy wiedziec, kiedy mozna CZYTAC
    fds[0].revents = 0;     // To pole wypelni sam system podczas dzialania

    // Przyklad B: pusty slot (np. czekajacy na jakis potok przypisany pozniej)
    fds[1].fd = -1; // Ustawienie fd na ujemna liczbe sprawia, ze poll CALKOWICIE go ignoruje
    fds[1].events = POLLIN;
    fds[1].revents = 0;

    // Liczba aktywnych deskryptorow do obsluzenia (zeby wiedziec, kiedy przerwac glowna petle)
    int active_fds = 1; 

    // =================================================================
    // 2. GŁÓWNA PĘTLA ZDARZEŃ (EVENT LOOP)
    // =================================================================
    while (active_fds > 0)
    {
        
        // poll(tablica, liczba_elementow, timeout_w_milisekundach)
        // timeout: -1 (czeka w nieskonczonosc), 0 (sprawdza i wraca natychmiast)
        int ret = poll(fds, MAX_FDS, -1);

        // Obsluga błędu przerwania przez sygnal (np. jakis SIGALRM w tle)
        if (ret < 0)
        {
            if (errno == EINTR) continue; // Wracamy do nasluchiwania
            ERR("poll");
        }
        
        // Timeout (jesli uzywamy timeout > 0)
        if (ret == 0)
        {
            printf("Minal czas oczekiwania, nic sie nie stalo...\n");
            continue;
        }

        // =================================================================
        // 3. PRZESZUKIWANIE TABLICY - KTO DOSTAL DANE?
        // =================================================================
        for (int i = 0; i < MAX_FDS; i++) {
            
            // Pomijamy zignorowane / zamkniete wczesniej deskryptory
            if (fds[i].fd < 0) continue;

            // A) ZDARZENIE: MOZNA CZYTAC LUB ROZLACZONO (ROZWIAZANIE "PROBLEMU BABCI")
            // Sprawdzamy POLLIN razem z POLLHUP, zeby nie zgubic "resztek" danych z zamknietej rury!
            if (fds[i].revents & (POLLIN | POLLHUP)) {
                
                char buf[256];
                ssize_t bytes_read = read(fds[i].fd, buf, sizeof(buf) - 1);
                
                if (bytes_read < 0) {
                    ERR("read");
                } 
                else if (bytes_read == 0)
                {
                    // Twardy dowod (EOF). 
                    // Dopiero teraz, gdy read zwroci 0, mamy matematyczna pewnosc, 
                    // ze odczytalismy wszystko i rura jest pusta.
                    printf("Deskryptor %d osiagnal EOF (bezpiecznie zamknieto).\n", fds[i].fd);
                    close(fds[i].fd);
                    fds[i].fd = -1; // Wylaczamy go z nasluchiwania
                    active_fds--;
                } 
                else
                {
                    // Mamy dane! Nawet jesli jest zapalony POLLHUP, na razie NIE zamykamy fd.
                    // Przy kolejnym obrocie petli znowu tu wejdziemy, az read w koncu zwroci 0.
                    buf[bytes_read] = '\0';
                    printf("Odebrano z fd %d: %s\n", fds[i].fd, buf);
                }
            }

            // B) AWARIE: POLLERR (Błąd sprzetowy) / POLLNVAL (Nieprawidlowy deskryptor)
            // Tu nie ma szans na żadne dane, wiec zamykamy natychmiast (else if!).
            else if (fds[i].revents & (POLLERR | POLLNVAL))
            {
                printf("Krytyczna awaria na fd %d.\n", fds[i].fd);
                close(fds[i].fd);
                fds[i].fd = -1;
                active_fds--;
            }
        }
    }

    printf("Wszystkie deskryptory zamkniete. Koniec programu.\n");
    return EXIT_SUCCESS;
}