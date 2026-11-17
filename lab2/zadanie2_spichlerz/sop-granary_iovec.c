#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/uio.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

// =====================================================================
// STRUKTURA PLIKU BINARNEGO (MAGAZYNU) NA DYSKU
// Plik to jeden zwarty, ciagly blok danych (jak pociag), nie ma spacji
//
// [NAGLOWEK - 8 bajtow] (Lokomotywa - zawsze jedna, na samym poczatku)
//   - 4 bajty: "SPI1" (magiczny napis, identyfikator pliku)
//   - 4 bajty: liczba N (ile roznych towarow jest zapisanych dalej)
//
// [REKORD 1 - 36 bajtow] (Wagon nr 1 - dopiety zaraz za naglowkiem)
//   - 32 bajty: nazwa towaru (np. "sol", reszta z 32 bajtow to zera \0)
//   - 4 bajty: ilosc (np. 150)
//
// [REKORD 2 - 36 bajtow] (Wagon nr 2) ... itd. az do rekordu N
// =====================================================================

// Struktura na naglowek pliku (dokladnie 8 bajtow)
typedef struct
{
    char magic[4];  // Ma trzymac napis "SPI1"
    uint32_t count; // Liczba rekordow w magazynie
} StorageHeader;

// Struktura na pojedynczy towar (dokladnie 36 bajtow)
typedef struct
{
    char name[32]; // Nazwa towaru (zawsze dopelniona zerami)
    uint32_t qty;  // Ilosc towaru
} StorageRecord;
// Wielkosc struktur dzieli sie przez 4, wiec nie bedzie paddingu

// Funkcja porownujaca dla qsort (sortowanie alfabetyczne towarow)
int compare_records(const void *a, const void *b)
{
    // Rzutowanie na wskazniki do struktur
    StorageRecord *recA = *(StorageRecord **)a;
    StorageRecord *recB = *(StorageRecord **)b;
    return strcmp(recA->name, recB->name);
}

char *trim(char *manifest)
{
    // Ucinaj spacje z lewej strony
    while (*manifest != '\0' && isspace(*manifest))
        manifest++;

    if (*manifest == '\0') // Jesli byl pusty lub skladal sie z samych spacji
        return manifest;

    // Ucinaj spacje z prawej strony
    char *end = manifest + strlen(manifest) - 1;
    while (end > manifest && isspace(*end))
    {
        *end = '\0';
        end--;
    }

    return manifest;
}

// Zmiana z main na funkcje parsujaca! Dodano flage 'silent'
int parse_manifest(const char *filepath, char *out_id, char *out_port,
                   char out_products[8][32], uint32_t out_amounts[8], int *out_count, int silent)
{
    // (Etap 1) Ten program idzie za linkiem symbolicznym i jesli to, co sie pod nim kryje jest manifestem to liczy
    int fd;
    if ((fd = open(filepath, O_RDONLY)) == -1)
        return -1; // Zamiast błędu po prostu pomijamy niedostępny plik

    struct stat file_stat;
    if ((fstat(fd, &file_stat)) == -1)
        ERR("fstat");
    if (!S_ISREG(file_stat.st_mode)) // Czy plik jest regularnym plikiem
    {
        if (!silent)
            fprintf(stderr, "%s to nie manifest: nie jest zwyklym plikiem\n", filepath);
        close(fd);
        return -1;
    }

    if (file_stat.st_size > 1023) // Czy plik nie jest za dlugi
    {
        if (!silent)
            fprintf(stderr, "%s to nie manifest: plik jest za duzy\n", filepath);
        close(fd);
        return -1;
    }
    char manifest[1024];
    ssize_t bytes_read = 0;
    if ((bytes_read = read(fd, manifest, 1023)) == -1)
        ERR("read");
    manifest[bytes_read] = '\0'; // Plik zapisany do bufora

    // Przesuniete zamykanie pliku z samego dolu (zeby uniknac wyciekow przy wczesnym return -1)
    if (close(fd))
        ERR("close");

    if (strlen(manifest) > 0 && manifest[strlen(manifest) - 1] == '\n')
        manifest[strlen(manifest) - 1] = '\0'; // Usuwamy opcjonalny enter z konca
    if (strchr(manifest, '\n') != NULL)        // Czy zawiera dwie linie
    {
        if (!silent)
            fprintf(stderr, "%s to nie manifest: zawiera wiecej niz jedna linie\n", filepath);
        return -1;
    }

    // Liczenie srednikow
    int semicolon_count = 0;
    for (int i = 0; i < strlen(manifest); i++)
    {
        if (manifest[i] == ';')
            semicolon_count++;
    }

    if (semicolon_count != 2)
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: nieprawidlowa liczba srednikow\n", filepath);
        return -1;
    }

    // Wyciagam rekordy
    char *id = strtok(manifest, ";");
    if (id == NULL)
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s\n", filepath);
        return -1;
    }

    id = trim(id);
    if (!(strlen(id) == 5 && isupper(id[0]) && id[1] == '-' && isdigit(id[2]) && isdigit(id[3]) && isdigit(id[4])))
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: brak id\n", filepath);
        return -1;
    }

    char *port = strtok(NULL, ";");
    if (port == NULL)
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: brak nazwy portu\n", filepath);
        return -1;
    }

    port = trim(port);
    if (!(strlen(port) >= 1 && strlen(port) <= 31 && strchr(port, '/') == NULL && strcmp(port, ".") != 0 && strcmp(port, "..") != 0))
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: nazwa portu nieprawidlowa\n", filepath);
        return -1;
    }

    char *position = strtok(NULL, ";");
    if (position == NULL)
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: brak towarow\n", filepath);
        return -1;
    }

    char *rest = strtok(NULL, ";");
    if (rest != NULL)
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: za duzo pol\n", filepath);
        return -1;
    }

    position = trim(position);

    // Liczenie przecinkow
    int comma_count = 0;
    for (int i = 0; i < strlen(position); i++)
    {
        if (position[i] == ',')
            comma_count++;
    }

    char *internal_save, *external_save;
    char *record = strtok_r(position, ",", &external_save);
    int record_count = 0;

    while (record != NULL)
    {
        record_count++;
        record = trim(record);

        if (record_count > 8)
        {
            if (!silent)
                fprintf(stderr, "Nieczytelny manifest %s: zla liczba pozycji\n", filepath);
            return -1;
        }

        // Liczenie dwukropkow
        int colon_count = 0;
        for (int i = 0; i < strlen(record); i++)
        {
            if (record[i] == ':')
                colon_count++;
        }
        if (colon_count != 1)
        {
            if (!silent)
                fprintf(stderr, "Nieczytelny manifest %s: nieprawidlowa liczba dwukropkow\n", filepath);
            return -1;
        }

        char *product = strtok_r(record, ":", &internal_save);
        char *amount = strtok_r(NULL, ":", &internal_save);

        if (amount == NULL) // Bylo zboze lub zboze: lub ::
        {
            if (!silent)
                fprintf(stderr, "Nieczytelny manifest %s: zla wartosc towar:ilosc\n", filepath);
            return -1;
        }

        product = trim(product);
        amount = trim(amount);

        // WALIDACJA TOWARU
        size_t product_length = strlen(product);
        if (product_length < 1 || product_length > 31)
        {
            if (!silent)
                fprintf(stderr, "Nieczytelny manifest %s: zla dlugosc towaru (%s)\n", filepath, product);
            return -1;
        }

        for (int i = 0; i < product_length; i++)
        {
            // islower() sprawdza małe litery a-z, isdigit() cyfry
            if (!(islower(product[i]) || isdigit(product[i]) || product[i] == ' '))
            {
                if (!silent)
                    fprintf(stderr, "Nieczytelny manifest %s: niedozwolone znaki w towarze (%s)\n", filepath, product);
                return -1;
            }
        }

        // WALIDACJA ILOSCI
        if (strlen(amount) == 0) // Czy w ogole cos tam jest
        {
            if (!silent)
                fprintf(stderr, "Nieczytelny manifest %s: brak ilosci\n", filepath);
            return -1;
        }

        for (int i = 0; i < strlen(amount); i++) // Czy kazdy znak to cyfra
        {
            if (!isdigit(amount[i]))
            {
                if (!silent)
                    fprintf(stderr, "Nieczytelny manifest %s: ilosc to nie tylko cyfry (%s)\n", filepath, amount);
                return -1;
            }
        }

        long value = atol(amount);

        if (value < 1 || value > 1000000) // Czy miesci sie w zakresie
        {
            if (!silent)
                fprintf(stderr, "Nieczytelny manifest %s: ilosc poza zakresem (%ld)\n", filepath, value);
            return -1;
        }

        // PAKUJEMY DO WIADEREK WYJSCIOWYCH
        if (out_products != NULL && out_amounts != NULL)
        {
            strncpy(out_products[record_count - 1], product, 31);
            out_products[record_count - 1][31] = '\0'; // Zabezpieczenie zera
            out_amounts[record_count - 1] = (uint32_t)value;
        }

        record = strtok_r(NULL, ",", &external_save);
    }

    if (record_count < 1) // Gorny limit jest sprawdzany wewnatrz petli
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: zla liczba pozycji\n", filepath);
        return -1;
    }

    if (comma_count != record_count - 1)
    {
        if (!silent)
            fprintf(stderr, "Nieczytelny manifest %s: puste pozycje (zla liczba przecinkow)\n", filepath);
        return -1;
    }

    // PRZEKAZANIE WYNIKOW NA ZEWNATRZ
    if (out_id != NULL)
        strcpy(out_id, id);
    if (out_port != NULL)
        strcpy(out_port, port);
    if (out_count != NULL)
        *out_count = record_count; // Przekazujemy ile towarow bylo na statku

    return 0; // Sukces
}

// Dzieki przekazaniu adresow created i unchanged mozemy je sobie modyfikowac
void make_link(const char *target, const char *linkpath, int *created, int *unchanged)
{
    struct stat link_stat;

    // Sprawdzamy czy pod scieka index/by-id/<ID> juz cos lezy
    if (lstat(linkpath, &link_stat) == 0) // Jesli udalo sie zapisac dane do stat, to cos lezy tu na dysku
    {
        if (S_ISLNK(link_stat.st_mode)) // Czy to link symboliczny?
        {
            char buf[PATH_MAX + 1];
            // readlink czyta na co wskazuje skrot, ale uwaga: NIE dopisuje '\0' na koncu!
            // readlink otwiera skrot, czyta sciezke docelowa, na ktora wskazuje i laduje do buf
            ssize_t len = readlink(linkpath, buf, sizeof(buf) - 1);
            if (len != -1) // Jesli readlink nie zakonczyl sie bledem
            {
                buf[len] = '\0'; // Musimy sami zamknac napis, readlink tego nie robi!

                // Jesli obecny cel skrotu jest dokladnie taki sam jak nasz nowy cel
                if (strcmp(buf, target) == 0)
                {
                    (*unchanged)++; // Zostawiamy bez zmian
                    return;
                }
            }
        }

        // Skoro kod doszedl tutaj, to znaczy, ze albo plik nie jest skrotem, albo skrot wskazuje na coc innego
        // Zatem jest konflikt (inny cel lub nie jest dowiazaniem)
        fprintf(stderr, "Konflikt w indeksie: %s\n", linkpath);
    }
    else
    {
        // lstat zwrocil blad, pod ta sciezka niczego nie ma, miejsce jest wolne
        if (symlink(target, linkpath) == -1)
            ERR("symlink");

        (*created)++;
    }
}

// Otwiera podany folder, przeglada plik po pliku
void remove_dangling(const char *dirpath, int *deleted)
{
    DIR *dir = opendir(dirpath);
    if (dir == NULL)
        return; // Blad po prostu

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) // Po kolei idziemy so plikach
    {
        // Pomijamy . i ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);

        struct stat link_stat, file_stat;

        // Sprawdzamy czy to jest dowiazanie (drogowskaz)
        if (lstat(path, &link_stat) == 0 && S_ISLNK(link_stat.st_mode))
        {
            // Podazamy za linkiem, zeby sprawdzic czy cel zniknal
            if (stat(path, &file_stat) == -1 && errno == ENOENT)
            {
                unlink(path); // Usuwamy wiszacy skrot
                (*deleted)++;
            }
        }
    }

    if (closedir(dir) == -1)
        ERR("closedir");
}

int main(int argc, char **argv)
{
    // -------------------------------------------------------------------
    // ETAP 3: Zbiorczy magazyn binarny (readv/writev)
    // Zarzadza plikiem z argumentu wywolania
    // Sciezka A (pusty): agreguje towary ze statkow i zapisuje (writev)
    // Sciezka B (istnieje): sprawdza metadane (fstat) i wczytuje (readv)
    // -------------------------------------------------------------------

    // Sprawdzamy czy uzytkownik podal sciezke do magazynu (nasz nowy argument)
    if (argc != 2)
    {
        fprintf(stderr, "Uzycie: %s <plik_magazynu>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Otwieramy plik do odczytu i zapisu, jesli nie ma - tworzymy z prawami 0644
    int mag_fd = open(argv[1], O_RDWR | O_CREAT, 0644);
    if (mag_fd < 0)
        ERR("open");

    // Struktura na metadane pliku
    struct stat mag_stat;
    if (fstat(mag_fd, &mag_stat) < 0)
        ERR("fstat");

    StorageRecord *records[100]; // Tablica wskaznikow na osobne struktury, tutaj nic jeszcze nie ma w sumie
    int unique_items = 0;        // Ilosc roznych towarow (miejsc w tablicy, ktore faktycznie cos maja)

    // Sprawdzamy czy plik magazynu to swiezak, czy juz tu lezal
    if (mag_stat.st_size == 0)
    {
        // ---------------------------------------------------------------------
        // SCIEZKA A: Tworzymy nowy magazyn i wypelniamy go danymi z manifestow
        // WRITEV (ZAPISYWANIE NA DYSK)
        // ---------------------------------------------------------------------

        DIR *dir;
        if ((dir = opendir("manifests")) != NULL)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                char filepath[PATH_MAX];
                snprintf(filepath, sizeof(filepath), "manifests/%s", entry->d_name);

                char id[6], port[32];
                char products[8][32]; // Tu parser wrzuci nazwy z 1 statku
                uint32_t amounts[8];  // Tu parser wrzuci ilosci z 1 statku
                int count = 0;        // Tu parser wrzuci ile bylo pozycji

                // Czytamy statek - TUTAJ CHCEMY BLEDY, wiec silent = 0
                if (parse_manifest(filepath, id, port, products, amounts, &count, 0) == 0)
                {
                    // Statek jest poprawny, wyciagamy z niego towar po towarze
                    for (int k = 0; k < count; k++)
                    {
                        // Sprawdzamy, czy ten towar juz mamy na ktorejs smyczy w records
                        int found = 0;
                        for (int i = 0; i < unique_items; i++)
                        {
                            if (strcmp(records[i]->name, products[k]) == 0)
                            {
                                // Mamy go! Zwiekszamy tylko ilosc
                                records[i]->qty += amounts[k];
                                found = 1;
                                break;
                            }
                        }

                        // Jeśli nie znalezlismy (nowy towar), tworzymy nowy obiekt
                        if (found == 0)
                        {
                            // Uzywamy calloc(1, rozmiar). 1 oznacza "jedna sztuka".
                            // Calloc od razu wypelnia caly ten plac zerami!
                            records[unique_items] = calloc(1, sizeof(StorageRecord));
                            if (!records[unique_items])
                                ERR("calloc");

                            // strncpy wypelni reszte ucietej tablicy name zerami
                            strncpy(records[unique_items]->name, products[k], 32);
                            records[unique_items]->qty = amounts[k];

                            unique_items++; // Zwiekszamy ogolny licznik towarow w magazynie
                        }
                    }
                }
            }
            closedir(dir);
        }

        // Sortujemy tablice wskaznikow rosnaco przed zapisem
        qsort(records, unique_items, sizeof(StorageRecord *), compare_records);

        // 1. Budujemy naglowek w pamieci
        StorageHeader header;
        memcpy(header.magic, "SPI1", 4);
        header.count = unique_items;

        // 2. Przygotowujemy wektory do writev
        struct iovec iov[101];

        iov[0].iov_base = &header;
        iov[0].iov_len = sizeof(StorageHeader);

        for (int i = 0; i < unique_items; i++)
        {
            iov[i + 1].iov_base = records[i];
            iov[i + 1].iov_len = sizeof(StorageRecord);
        }

        // 3. Zbierz wszystkie czesci i zapisz ciurkiem na dysk (gather)
        if (writev(mag_fd, iov, 1 + unique_items) == -1)
            ERR("writev");

        // 4. Przewijamy kursor na poczatek pliku, bo zaraz bedziemy z niego czytac
        if (lseek(mag_fd, 0, SEEK_SET) == -1)
            ERR("lseek");
    }
    else
    {
        // -----------------------------------------------------------------
        // SCIEZKA B: Plik istnieje, sprawdzamy czy to w ogole jest magazyn
        // READV (CZYTANIE Z PLIKU)
        // -----------------------------------------------------------------

        // Z polecenia: naglowek to 8 bajtow, reszta dzieli sie przez 36 bajtow (rozmiar rekordu)
        if (mag_stat.st_size < 8 || (mag_stat.st_size - 8) % 36 != 0)
        {
            fprintf(stderr, "To nie jest magazyn!\n");
            exit(EXIT_FAILURE);
        }

        // Znamy rozmiar z mag_stat, wiec wyliczamy ile jest towarow (naszych wagonow)
        unique_items = (mag_stat.st_size - 8) / 36;

        // Alokujemy w pamieci te "osobne obiekty" zeby readv mial gdzie wrzucac dane
        for (int i = 0; i < unique_items; i++)
        {
            records[i] = malloc(sizeof(StorageRecord));
            if (!records[i])
                ERR("malloc");
        }
    }

    StorageHeader read_header;
    struct iovec iov_read[101];

    // Szykujemy wektory pod odczyt z pliku do pamieci
    iov_read[0].iov_base = &read_header;
    iov_read[0].iov_len = sizeof(StorageHeader);

    for (int i = 0; i < unique_items; i++)
    {
        iov_read[i + 1].iov_base = records[i];
        iov_read[i + 1].iov_len = sizeof(StorageRecord);
    }

    // Strzelamy odczytem (scatter - rozrzuca dane z pliku po naszych adresach)
    if (readv(mag_fd, iov_read, 1 + unique_items) == -1)
        ERR("readv");

    // Ostatnia walidacja z polecenia: czy magic to "SPI1" i czy count sie zgadza
    if (strncmp(read_header.magic, "SPI1", 4) != 0 || read_header.count != unique_items)
    {
        fprintf(stderr, "To nie jest magazyn!\n");
        exit(EXIT_FAILURE);
    }

    // Wypisywanie koncowe z polecenia
    printf("Stan magazynu, towarow: %u\n", read_header.count);
    for (int i = 0; i < unique_items; i++)
    {
        // Wydruk towaru, nazwa jest zapelniona zerami, wiec printf("%s") jest bezpieczny
        printf("  %s: %u\n", records[i]->name, records[i]->qty);
    }

    // ----------------------------------------------------------
    // ETAP 2 : Indeksowanie statkow i sprzatanie skrotow
    // Przeszukuje folder 'manifests', czyta dane kazdego statku
    // i robi mu dowiazania w 'index/by-id' i 'index/by-port'
    // Na koniec usuwa skroty do statkow, ktore juz odplynely
    // ----------------------------------------------------------
    int stat_created = 0,
        stat_unchanged = 0, stat_deleted = 0;

    // Tworzenie katalogow
    mkdir("index", 0755); // Jesli juz istnieje, to po prostu sie nie tworzy i wywalone
    mkdir("index/by-id", 0755);
    mkdir("index/by-port", 0755);

    DIR *dir;
    if ((dir = opendir("manifests")) == NULL)
        ERR("opendir");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Pomijamy . i ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Tworzymy sciezke do pliku
        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "manifests/%s", entry->d_name);

        struct stat link_stat;
        if (lstat(filepath, &link_stat) == -1)
            continue; // Idziemy dalej, nie ubijamy

        if (S_ISREG(link_stat.st_mode)) // Dzieki lstat nie idziemy za skrotem
        {
            char id[6], port[32];

            // Wywolanie parse_manifest z flagą silent = 1 (TUTAJ JUZ NIE CHCEMY BLEDOW)
            if (parse_manifest(filepath, id, port, NULL, NULL, NULL, 1) == 0)
            {
                // BY ID
                char linkpath_id[PATH_MAX], target_id[PATH_MAX];

                // Gdzie ma lezec skrot? np. "index/by-id/K-017"
                snprintf(linkpath_id, sizeof(linkpath_id), "index/by-id/%s", id);

                // Na co ma wskazywac? np. "../../manifests/galeon.txt"
                snprintf(target_id, sizeof(target_id), "../../manifests/%s", entry->d_name);

                // Zrob dowiazanie
                make_link(target_id, linkpath_id, &stat_created, &stat_unchanged);

                // BY PORT and BY ID
                char port_dir[PATH_MAX], linkpath_port[PATH_MAX], target_port[PATH_MAX];

                // Sklejamy nazwe katalogu portu, np. "index/by-port/Gdynia"
                snprintf(port_dir, sizeof(port_dir), "index/by-port/%s", port);

                // Tworzymy ten podkatalog (ignorujemy bledy, np. jesli ktos juz wczesniej wyslal statek do Gdyni)
                mkdir(port_dir, 0755);

                // Sklejamy miejsce naszego skrotu, np. "index/by-port/Gdynia/K-017"
                snprintf(linkpath_port, sizeof(linkpath_port), "index/by-port/%s/%s", port, id);

                // Wyliczamy cel skrotu (3 poziomy w gore!), np. "../../../manifests/galeon.txt"
                snprintf(target_port, sizeof(target_port), "../../../manifests/%s", entry->d_name);

                // Uzywamy naszej kuloodpornej funkcji do zrobienia linku
                make_link(target_port, linkpath_port, &stat_created, &stat_unchanged);
            }
        }
    }

    if (closedir(dir))
        ERR("closedir");

    // Sprzatanie wiszacych linkow
    // Sprzatamy folder z BY ID
    remove_dangling("index/by-id", &stat_deleted);

    // Sprzatamy katalogi portow BY PORT and BY ID
    DIR *port_dir;
    if ((port_dir = opendir("index/by-port")) == NULL)
        ERR("opendir");

    struct dirent *p_entry;
    while ((p_entry = readdir(port_dir)) != NULL)
    {
        // Pomijamy . i ..
        if (strcmp(p_entry->d_name, ".") == 0 || strcmp(p_entry->d_name, "..") == 0)
            continue;

        // Kleimy sciezke do konkretnego portu, np. "index/by-port/Gdynia"
        char subpath[PATH_MAX];
        snprintf(subpath, sizeof(subpath), "index/by-port/%s", p_entry->d_name);

        // Wchodzimy do portu i sprzatamy w srodku
        remove_dangling(subpath, &stat_deleted);
    }

    if (closedir(port_dir))
        ERR("closedir");

    fprintf(stdout, "Indeks: utworzono %d, bez zmian %d, usunięto %d\n", stat_created, stat_unchanged, stat_deleted);

    // Czyszczenie pamieci
    for (int i = 0; i < unique_items; i++)
    {
        free(records[i]);
    }
    close(mag_fd);

    return EXIT_SUCCESS;
}