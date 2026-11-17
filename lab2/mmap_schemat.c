#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int main()
{
    const char *filepath = "baza.bin";

    // 1. Otwarcie pliku (musi miec prawa odpowiadajace mapowaniu)
    int fd = open(filepath, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
        ERR("open");

    // 2. Pobranie rozmiaru pliku (st.st_size)
    struct stat st;
    if (fstat(fd, &st) < 0) ERR("fstat");

    size_t length = st.st_size;

    // 3. KLUCZOWE: Rozciąganie nowego pliku!
    // Jeśli plik ma 0 bajtów, mmap sie powiedzie, ale ZAPIS wywali SIGBUS (Bus Error)!
    if (length == 0)
    {
        length = 1024; // Ustawiamy oczekiwany rozmiar pliku (np. 1024 bajty)
        if (ftruncate(fd, length) == -1) ERR("ftruncate");
    }

    // 4. Mapowanie w pamiec
    // mmap(gdzie_zamapowac, dlugosc, prawa_dostepu, flagi, deskryptor, offset)
    void *mapped = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    // UWAGA: mmap nie zwraca NULL w razie bledu, tylko MAP_FAILED!
    if (mapped == MAP_FAILED)
        ERR("mmap");

    // Od razu po mapowaniu mozna bezpiecznie zamknac plik! (Mapowanie zostaje aktywne)
    if (close(fd))
        ERR("close");

    // =========================================================
    // DZIAŁANIE NA DANYCH (Traktujemy plik jak pamiec RAM)
    // =========================================================
    char *data = (char *)mapped;
    data[0] = 'H';
    data[1] = 'E';
    data[2] = 'J'; // Zmiany leca prosto do pliku!

    // =========================================================
    // 5. Sprzatanie na sam koniec
    // =========================================================
    
    // Wymuszenie fizycznego zapisu na dysk (opcjonalne, ale bardzo dobre na labach)
    if (msync(mapped, length, MS_SYNC) == -1)
        ERR("msync");

    // Usuniecie mapowania
    if (munmap(mapped, length) == -1)
        ERR("munmap");

    return EXIT_SUCCESS;
}