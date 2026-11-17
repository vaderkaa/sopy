#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

// ========================================================================
// STRUKTURY IOVEC TO "MAPA" PAMIECI (adres i dlugosc bloku)
//
// WRITEV (Gather): System biegnie pod adresy z mapy, zbiera z nich dane 
// i zrzuca je w jednym ciaglym bloku do pliku na dysk.
//
// READV (Scatter): System bierze jeden ciagly blok z pliku na dysku, 
// tnie go na podane wymiary i rozrzuca pod wskazane adresy w pamieci RAM.
// ========================================================================

typedef struct
{
    char magic[4];
    uint32_t count;
} KwitHeader;

int main(int argc, char **argv) 
{
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s plik [tytul n1 ... n10]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filepath = argv[1];

    if (argc >= 4 && argc <= 13)
    {
        // ZAPIS, A POTEM ODCZYT
        // Uruchomiono ./sop-kwit plik tytul n1 [n2 ... n10]
        uint32_t N = argc - 3;

        // 1. Otwieramy plik do ZAPISU (O_WRONLY)
        // O_CREAT: jak nie ma, to utworz
        // O_TRUNC: jak juz jest i ma stare dane, to go wyczysc (wyzeruj rozmiar)
        int fd_write = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_write < 0)
            ERR("open write");

        // 2. Przygotowujemy nasze trzy zmienne
        
        // Zmienna A: Naglowek
        KwitHeader write_header;
        memcpy(write_header.magic, "KWIT", 4);
        write_header.count = N;

        // Zmienna B: Tytul (musimy dopilnowac zer!)
        char write_title[32];
        memset(write_title, 0, 32); // Najpierw calosc zalewamy zerami
        // Kopiujemy tytul z argv[2]. Limit to 31, zeby zawsze zostalo jakies zero
        strncpy(write_title, argv[2], 31);

        // Zmienna C: Tablica na pozycje
        uint32_t amounts_write[10]; // Wiemy z polecenia, ze maks to 10 elementow
        for (int i = 0; i < N; i++)
        {
            // atoi zmienia tekst z argv na liczbe. Zaczynamy od argv[3]
            amounts_write[i] = (uint32_t)atoi(argv[3 + i]); 
        }

        // 3. Pakujemy zmienne do wektorow dla kuriera (writev)
        struct iovec iov_write[3];

        iov_write[0].iov_base = &write_header;
        iov_write[0].iov_len = sizeof(KwitHeader); // 8 B

        iov_write[1].iov_base = write_title;
        iov_write[1].iov_len = 32; // 32 B

        iov_write[2].iov_base = amounts_write;
        iov_write[2].iov_len = N * sizeof(uint32_t); // N * 4 B

        // 4. Strzal na dysk jednym poleceniem
        ssize_t expected_size = 40 + (N * 4);
        if (writev(fd_write, iov_write, 3) != expected_size)
            ERR("writev");

        // Konczymy pisanie i zamykamy plik
        // Dzieki temu dolna czesc programu bedzie mogla go otworzyc w O_RDONLY do odczytu
        if (close(fd_write))
            ERR("close write");
    }
    else
    {
        fprintf(stderr, "Nieprawidlowa liczba argumentow (za duzo liczb lub brak liczb po tytule).\n");
        exit(EXIT_FAILURE);
    }

    // Wspolna czesc - odczyt READV i wypisywanie

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) 
        ERR("open");

    struct stat st;
    if (fstat(fd, &st) < 0) 
        ERR("fstat");

    // 1. Walidacja rozmiaru i wyliczanie N
    // Naglowek(8) + Tytul(32) = 40 bajtow bazy
    // Reszta to wielokrotnosci 4 bajtow
    if (st.st_size < 44 || (st.st_size - 40) % 4 != 0)
    {
        fprintf(stderr, "To nie kwit!\n");
        exit(EXIT_FAILURE);
    }

    // Wyliczamy liczbe pozycji na podstawie wielkosci pliku (a nie z argumentow)
    uint32_t N = (st.st_size - 40) / 4;

    // 2. Przygotowanie 3 osobnych zmiennych / buforow do odczytu
    KwitHeader read_header;
    
    char read_title[33];    // 32 bajty z pliku + 1 bajt na zero
    read_title[32] = '\0';  // Twarde zakonczenie zerem (zabezpieczenie dla printf)
    
    uint32_t *amounts = malloc(N * sizeof(uint32_t));
    if (!amounts)
        ERR("malloc");

    // 3. Wektory dla readv (dokladnie 3 elementy)
    struct iovec iov_read[3];

    iov_read[0].iov_base = &read_header;
    iov_read[0].iov_len = sizeof(KwitHeader); // 8 B

    iov_read[1].iov_base = read_title;
    iov_read[1].iov_len = 32; // rowne 32 B, nie nadpisze naszego '\0' na koncu

    iov_read[2].iov_base = amounts;
    iov_read[2].iov_len = N * sizeof(uint32_t); // 4 * N B

    // 4. Odczyt jednym strzalem
    if (readv(fd, iov_read, 3) != st.st_size)
        ERR("readv");

    if (close(fd)) 
        ERR("close");

    // 5. Walidacja wnetrza kwitu
    if (strncmp(read_header.magic, "KWIT", 4) != 0 || read_header.count != N)
    {
        fprintf(stderr, "To nie kwit!\n");
        free(amounts); // Sprzatamy pamiec przed awaryjnym wyjsciem
        exit(EXIT_FAILURE);
    }

    // 6. Wypisywanie koncowe
    printf("DEBUG Naglowek -> Magic: '%.4s', Count: %u\n", read_header.magic, read_header.count);
    printf("Tytul: %s\n", read_title);
    
    printf("Pozycje:");
    uint32_t sum = 0;
    for (uint32_t i = 0; i < N; i++)
    {
        printf(" %u", amounts[i]);
        sum += amounts[i];
    }
    printf("\nSuma: %u\n", sum);

    free(amounts);

    return EXIT_SUCCESS;
}