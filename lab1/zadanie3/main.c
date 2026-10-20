#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "Użycie:\n");
    fprintf(stderr, "  Tworzenie: %s -c -v <nazwa_srodowiska>\n", pname);
    exit(EXIT_FAILURE);
}

void create_environment(const char *env_name)
{
    // 1. Tworzenie katalogu (0777 to standardowe pełne uprawnienia, które system sam przytnie umaskiem)
    // Daje katalogowi najwyższe uprawnienia, na jakie aktualnie pozwala system
    if (mkdir(env_name, 0777) == -1)
    {
        perror("mkdir");
        return; // Zwracamy kontrolę zamiast ubijać program
    }

    // 2. Sklejanie ścieżki do pliku requirements (np. "moje_srodowisko/requirements")
    char req_path[PATH_MAX];
    snprintf(req_path, sizeof(req_path), "%s/requirements", env_name);
    // Gdzie trafi tekst, ile maksymalnie go trafi, co trafi i co podstawiamy

    // 3. Tworzenie pustego pliku tekstowego w trybie do zapisu ("w")
    FILE *f = fopen(req_path, "w");
    if (f == NULL)
    {
        perror("fopen requirements");
        return;
    }

    if (fclose(f))
        ERR("fclose");
}

void install_package(const char *env_name, char *pkg_info)
{
    // 1. Rozdzielanie nazwy i wersji (szukamy znaku "==")
    char *eq = strstr(pkg_info, "==");
    if (eq == NULL)
    {
        fprintf(stderr, "Błąd: Zły format pakietu. Użyj nazwa==wersja\n");
        exit(EXIT_FAILURE); // Błąd w składni przerywa program
    }

    *eq = '\0'; // Zastępujemy pierwsze '=' znakiem końca stringa, odcinając nazwę
    char *pkg_name = pkg_info;
    char *pkg_version = eq + 2; // Wersja zaczyna się dwa znaki za pierwszym '='

    // Sklejamy ścieżkę do pliku requirements
    char req_path[PATH_MAX];
    snprintf(req_path, sizeof(req_path), "%s/requirements", env_name);

    // 2. Sprawdzanie duplikatów ("a+" pozwala na czytanie i dopisywanie na końcu)
    FILE *f = fopen(req_path, "a+");
    if (f == NULL)
    {
        fprintf(stderr, "sop-venv: the environment does not exist\n");
        exit(EXIT_FAILURE);
    }

    rewind(f); // Cofamy wskaźnik na początek pliku, żeby go przeczytać
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, f)) != -1)
    {
        // Sprawdzamy, czy linijka zaczyna się od nazwy naszego pakietu i spacji (np. "numpy ")
        if (strncmp(line, pkg_name, strlen(pkg_name)) == 0 && line[strlen(pkg_name)] == ' ')
        {
            fprintf(stderr, "Błąd: Pakiet %s jest już zainstalowany.\n", pkg_name);
            free(line);
            fclose(f);
            exit(EXIT_FAILURE);
        }
    }
    free(line);

    // 3. Dopisujemy nowy pakiet do pliku (w trybie "a+" zapis zawsze leci na sam koniec)
    fprintf(f, "%s %s\n", pkg_name, pkg_version);
    if (fclose(f))
        ERR("fclose requirements");

    // 4. Tworzenie fizycznego pliku pakietu z uprawnieniami tylko do odczytu (0444)
    char pkg_path[PATH_MAX];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s", env_name, pkg_name);

    int fd = open(pkg_path, O_CREAT | O_WRONLY | O_TRUNC, 0444);
    if (fd == -1)
        ERR("open package file");

    // Generowanie losowych znaków (np. 20 wielkich liter)
    char random_data[20];
    for (int i = 0; i < 20; i++)
    {
        random_data[i] = 'A' + (rand() % 26);
    }

    if (write(fd, random_data, 20) != 20)
        ERR("write");
    if (close(fd))
        ERR("close package file");
}

int main(int argc, char **argv)
{
    int c;
    int create_flag = 0;
    char *env_name = NULL;
    char *install_pkg = NULL; // Nowa zmienna dla Etapu 2

    // Pobieramy flagi (c nie ma dwukropka, bo to tylko przełącznik. v, i, r pobierają parametry)
    while ((c = getopt(argc, argv, "cv:i:r:")) != -1)
    {
        switch (c)
        {
        case 'c':
            create_flag = 1;
            break;
        case 'v':
            env_name = optarg;
            break;
        case 'i':
            install_pkg = optarg; // Zapisujemy paczkę do instalacji
            break;
        case '?':
        default:
            usage(argv[0]);
        }
    }

    // Prowadzący w tekście pisze -c -v nazwa, ale w przykładzie używa -c nazwa
    // Ten warunek wyłapie "luźną" nazwę, jeśli ktoś zapomni wpisać flagi -v
    // Jeśli nie podaliśmy -v, ale na końcu został luźny argument pozycyjny, to go bierzemy jako naszą nazwę
    if (env_name == NULL && optind < argc)
        env_name = argv[optind];

    if (env_name == NULL)
    {
        fprintf(stderr, "Błąd: Brak nazwy środowiska.\n");
        usage(argv[0]);
    }

    // Jeśli podano flagę -c (tworzenie)
    if (create_flag)
    {
        create_environment(env_name);
        return EXIT_SUCCESS; // Kończymy program po stworzeniu środowiska, bo tak każe polecenie
    }
    // Jeśli nie tworzymy, a podano flagę -i (instalacja)
    else if (install_pkg != NULL)
    {
        install_package(env_name, install_pkg);
    }

    return EXIT_SUCCESS;
}