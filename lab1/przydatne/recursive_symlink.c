// Rekurencyjne przeszukiwanie katalogu i tworzenie dowiązań symbolicznych (symlinków)
// Działanie krok po kroku:
// 1. Otwiera wskazany katalog główny (np. "biuro") za pomocą opendir()
// 2. Przegląda zawartość katalogu element po elemencie za pomocą readdir()
// 3. Pomija wpisy specjalne "." oraz ".."
// 4. Buduje pełną ścieżkę do każdego pliku/katalogu i pobiera jego metadane za pomocą lstat()
// 5. Jeśli napotka podkatalog, wywołuje funkcję rekurencyjnie (scan_dir), przechodząc w głąb struktury
// 6. (Opcjonalnie odkomentowane) Dla plików regularnych sprawdza ich ścieżki i tworzy odpowiednie
// dowiązania symboliczne (symlinki) do dedykowanych folderów archiwum
// 7. Po zakończeniu odczytu sprząta zasoby: sprawdza błędy readdir, zamyka katalog przez closedir()

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

// Makro do obsługi błędów systemowych (wypisuje błąd, plik, linię i kończy program)
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void scan_dir(char *path)
{
    DIR *dir;
    struct dirent *dp;
    struct stat st;

    // Otwarcie katalogu do odczytu
    if ((dir = opendir(path)) == NULL)
        ERR("opendir");

    errno = 0;
    // Pętla odczytująca zawartość katalogu element po elemencie
    while ((dp = readdir(dir)) != NULL)
    {
        // Pomijamy katalog bieżący (.) i nadrzędny (..)
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        // Sklejanie pełnej ścieżki do napotkanego elementu
        char file_path[1024] = "";
        strcat(file_path, path);
        strcat(file_path, "/");
        strcat(file_path, dp->d_name);

        // Pobieranie informacji o pliku/katalogu za pomocą lstat
        if (lstat(file_path, &st) == -1)
            ERR("lstat");

        // Jeśli element jest zwykłym plikiem
        if (S_ISREG(st.st_mode))
        {
            // Funkcja strstr sprawdza, czy w pełnej ścieżce (file_path) występuje słowo "ksiegowosc"
            char *s_k = strstr(file_path, "ksiegowosc");

            // Jeśli strstr nie zwróciło NULL, to znaczy, że słowo zostało znalezione
            if (s_k != NULL)
            {
                // Gdzie chcemy stworzyć skrót? (np. "archiwum/ksiegowe/faktura.txt")
                char link[1024] = "archiwum/ksiegowe/";
                strcat(link, dp->d_name); // Doklejamy samą nazwę pliku na koniec ścieżki

                // Dokąd ma prowadzić skrót?
                // Skoro skrót leży w "archiwum/ksiegowe/", musimy wyjść dwa foldery w górę ("../../")
                // Aby wrócić do głównego katalogu (projektu) i stamtąd wskazać oryginalny file_path
                char target[1024] = "../../";
                strcat(target, file_path);

                // Tworzymy dowiązanie symboliczne (skrót)
                // symlink(dokąd_prowadzi, gdzie_leży_skrót)
                if (symlink(target, link) == -1)
                    ERR("symlink ksiegowosc");
            }
        }
        // Jeśli element jest podkatalogiem – wchodzimy w niego rekurencyjnie
        else if (S_ISDIR(st.st_mode))
            scan_dir(file_path);

        errno = 0; // Zerowanie errno przed kolejnym obrotem readdir
    }

    // Sprawdzenie, czy pętla zakończyła się z powodu błędu odczytu
    if (errno != 0)
        ERR("readdir");

    // Zamknięcie otwartego strumienia katalogu
    if (closedir(dir) == -1)
        ERR("closedir");
}

int main(int argc, char **argv)
{
    // Uruchomienie przeszukiwania od katalogu "biuro"
    scan_dir("biuro");

    return EXIT_SUCCESS;
}