#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname) // jesli uzytkownik cos zle poda, to wyskakuje mu intrukcja
{
    fprintf(stderr, "USAGE:%s -p PATH_NAME -o OUTPUT_FILE\n", pname);
    exit(EXIT_FAILURE);
}

void listing(char *path, FILE *out_stream)
{
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;

    fprintf(out_stream, "ŚCIEŻKA: %s\n", path);

    if ((dirp = opendir(path)) == NULL)
    {
        perror(path); // wyswietlam blad
        return;       // wychodze z funkcji dla tego katalogu, ale ide dalej
    }

    fprintf(out_stream, "PLIKI:\n");

    do
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;

            if (fstatat(dirfd(dirp), dp->d_name, &filestat, AT_SYMLINK_NOFOLLOW)) // ZAPISUJE STRUKTURE PLIKU DO STRUMIENIA
                ERR("fstatat");

            fprintf(out_stream, "%s  ", dp->d_name);
            fprintf(out_stream, "%ld\n", filestat.st_size);
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");

    if (closedir(dirp))
        ERR("closedir");
}

int main(int argc, char **argv)
{
    int c;
    int listed = 0;
    int output = 0;
    FILE *out_stream = stdout;
    char expanded_path[PATH_MAX]; // do bonusu

    while ((c = getopt(argc, argv, "p:o:")) != -1)
    {
        if (c == 'o')
        {
            if (output == 0)
            {
                // taki bonus zeby dzialalo podmienianie sciezki \~/out.txt (moze sie przyda)
                char *file_path = optarg;

                if (file_path[0] == '~')
                {
                    char *home = getenv("HOME"); // pobieram sciezke domowa /home/jadzia
                    if (home == NULL)
                        ERR("getenv");

                    // sklejam /home/jadzia z reszta sciezki (optarg + 1 omija znak tyldy)
                    snprintf(expanded_path, sizeof(expanded_path), "%s%s", home, file_path + 1);
                    file_path = expanded_path; // podmieniam wskaznik na gotowa sciezke
                }

                if ((out_stream = fopen(file_path, "w+")) == NULL) // TWORZE SOBIE NOWY PLIK I WYBIERAM GO NA WYJSCIE
                    ERR("fopen");

                output = 1;
            }
            else
                usage(argv[0]);
        }
    }

    if (optind < argc) // jesli liczba rozpoznanych argumentow jest mniejsza niz liczba wszytkich
                       // to znaczy, ze sa dodatkowe parametry
        usage(argv[0]);

    optind = 1; // USTAWIAM SPOWROTEM, ZEBY MOC PRZECHODZIC JESZCZE RAZ

    while ((c = getopt(argc, argv, "p:o:")) != -1)
    {
        switch (c)
        {
        case 'p':
            listing(optarg, out_stream);
            listed = 1;
            break;
        case 'o': // MUSI TU STAC MIMO, ZE NIC NIBY NIE ROBI
            break;
        case '?':
        default:
            usage(argv[0]);
        }
    }

    if (listed == 0) // JESLI NIC NIE BYLO, TO URUCHAMIAM W KATALOGU ROBOCZYM
        listing(".", out_stream);

    if (out_stream != stdout) // JESLI OTWORZYLAM PLIK, TO MUSZE ZAMKNAC
        if (fclose(out_stream))
            ERR("fclose");

    return EXIT_SUCCESS;
}