// Usuwanie konkretnej linii z pliku tekstowego
// Otwiera plik źródłowy oraz plik tymczasowy, czyta plik linia po linii
// Przepisuje do pliku tymczasowego wszystkie linie POZA tą, która zaczyna się od szukanej nazwy (rev_name) i spacji
// Na koniec podmienia stary plik nowym

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Przykładowe makro obsługi błędów (często spotykane na labach z systemów operacyjnych)
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Użycie: %s <plik> <nazwa_do_usunięcia>\n", argv[1], argv[2]);
        exit(EXIT_FAILURE);
    }

    const char *path = argv[1];
    const char *rev_name = argv[2];

    // Otwarcie pliku źródłowego do odczytu
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        ERR("fopen fp");

    // Otwarcie pliku tymczasowego do zapisu
    FILE *fp_temp = fopen("temp.txt", "w");
    if (fp_temp == NULL)
    {
        fclose(fp);
        ERR("fopen fp_temp");
    }

    char *buf = NULL; // Funkcja getline sama zaalokuje pamiec
    size_t size = 0;  // Rozmiar bufora, getline sam ja zaktualizuje
    ssize_t ret;      // Wynik zwracany przez getline - liczba wczytanych znakow wraz ze znakiem nowej linii
    // lub -1 w przypadku bledu/konca pliku

    // Odczyt pliku linia po linii
    errno = 0;
    while ((ret = getline(&buf, &size, fp)) != -1)
    {
        // Jeśli linia zaczyna się od rev_name i następuje po niej spacja – pomijamy (usuwamy)
        if (strncmp(buf, rev_name, strlen(rev_name)) == 0 && buf[strlen(rev_name)] == ' ')
            continue;

        // W przeciwnym razie przepisujemy linię do pliku tymczasowego
        fprintf(fp_temp, "%s", buf);
    }

    if (errno != 0)
        ERR("getline");

    // Sprzątanie zasobów
    free(buf);
    fclose(fp);
    fclose(fp_temp);

    // Podmienienie starego pliku nowym (tymczasowym)
    if (rename("temp.txt", path) != 0)
        ERR("rename");

    printf("Pomyślnie usunięto wpis dla: %s\n", rev_name);
    return 0;
}