// Usuwanie pakietu ze środowiska (edycja pliku requirements i fizyczne usunięcie pliku)
// Działanie krok po kroku:
// 1. Tworzy ścieżkę i otwiera plik tymczasowy ("temp") w zadanym katalogu środowiska do zapisu
// 2. Otwiera oryginalny plik "requirements" do odczytu
// 3. Czyta plik requirements linia po linii za pomocą getline i przepisuje do pliku tymczasowego
// wszystkie linie, pomijając tę, która dotyczy usuwanego pakietu (rev_name)
// 4. Fizycznie usuwa z dysku plik reprezentujący ten pakiet w katalogu środowiska za pomocą unlink()
// 5. Zamyka oba pliki, usuwa stary plik requirements i podmienia go plikiem tymczasowym (rename)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// Makro obsługi błędów systemowych (wypisuje błąd, plik, linię i kończy program)
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Użycie: %s <katalog_srodowiska> <nazwa_pakietu> <sciezka_do_requirements>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Przypisanie argumentów do czytelnych zmiennych
    // W kontekście Twojego kodu: arr[i] to katalog środowiska, a rev_name to nazwa pakietu
    char *arr_i = argv[1];
    const char *rev_name = argv[2];
    const char *path = argv[3];

    // 1. Utworzenie pliku tymczasowego wewnątrz katalogu środowiska nazwa_srodowiska/temp
    char temp_path[1024] = "";
    strcat(temp_path, arr_i);
    strcat(temp_path, "/");
    strcat(temp_path, "temp");

    FILE *fp_temp = fopen(temp_path, "w");
    if (fp_temp == NULL)
        ERR("fopen fp_temp");

    // 2. Otwarcie oryginalnego pliku requirements do odczytu
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        ERR("fopen fp");

    // Zmienne do dynamicznego odczytu linii przez getline
    char *buf = NULL;
    size_t size = 0;
    ssize_t ret;

    // 3. Odczyt pliku linia po linii i przepisanie wszystkich poza usuwanym pakietem
    errno = 0;
    while ((ret = getline(&buf, &size, fp)) != -1)
    {
        // Jeśli linia pasuje do usuwanego pakietu (nazwa + spacja) – pomijamy ją (usuwamy)
        if (strncmp(buf, rev_name, strlen(rev_name)) == 0 && buf[strlen(rev_name)] == ' ')
            continue;

        // Pozostałe linie przepisujemy do pliku tymczasowego
        fprintf(fp_temp, "%s", buf);
    }

    // Zabezpieczenie sprawdzające błąd odczytu w pętli
    if (errno != 0)
        ERR("getline");

    free(buf);

    // 4. Fizyczne usunięcie pliku reprezentującego dany pakiet z dysku
    char remove_path[1024] = "";
    strcat(remove_path, arr_i);
    strcat(remove_path, "/");
    strcat(remove_path, rev_name);

    if (unlink(remove_path) == -1)
        ERR("unlink remove_path");

    // 5. Zamknięcie obu plików przed operacjami systemowymi na ich nazwach
    if (fclose(fp_temp) == -1)
        ERR("fclose fp_temp");

    if (fclose(fp) == -1)
        ERR("fclose fp");

    // 6. Podmiana starego pliku requirements nowym plikiem tymczasowym
    if (unlink(path) == -1)
        ERR("unlink path");

    if (rename(temp_path, path) != 0)
        ERR("rename");

    printf("Pomyślnie usunięto pakiet '%s' ze środowiska '%s'.\n", rev_name, arr_i);

    return EXIT_SUCCESS;
}