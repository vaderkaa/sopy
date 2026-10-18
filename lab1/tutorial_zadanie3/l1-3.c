#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname) // jesli uzytkownik cos zle poda, to wyskakuje mu intrukcja
{
    fprintf(stderr, "USAGE:%s -n Name -p OCTAL -s SIZE\n", pname);
    exit(EXIT_FAILURE);
}

void make_file(char *name, ssize_t size, mode_t perms, int percent)
{
    FILE *s1;
    int i;
    umask(~perms & 0777); // umask odbiera uprawnienia, wiec odwracamy perms i robimy koniunkcje i dostajemy to, co chcemy
    // nigdzie tego nie przypisujemy, uprawnienia sie same ustawia kiedy otworzymy plik fopen
    if ((s1 = fopen(name, "w")) == NULL) // fopen nadaje tylko prawa 0666
        ERR("fopen");
    for (i = 0; i < (size * percent) / 100; i++) // ile powinno byc losowych znakow A-Z
    {
        long large_random = ((long)rand() * (RAND_MAX + 1L)) + rand(); // DUZA LICZBA LOSOWA
        if (fseek(s1, large_random % size, SEEK_SET))                  // skok i zapis za koncem pliku powoduje wypelnienie luki \0
            ERR("fseek");
        fprintf(s1, "%c", 'A' + (i % ('Z' - 'A' + 1))); // i % dlugosc alfabetu, lepiej rand() % 26
    }
    if (ftruncate(fileno(s1), size)) // fileno(FILE *file) daje deskryptor do pliku, ftruncate(int filedes, off_t length) rozszerza
        ERR("ftruncate");
    if (fclose(s1))
        ERR("fclose");
}

int main(int argc, char **argv)
{
    int c;
    char *name = NULL; // inicjujemy bezsensownymi wartosciami, zeby sprawdzic czy uzytkownik podal cos sensownego
    mode_t perms = -1;
    ssize_t size = -1;
    while ((c = getopt(argc, argv, "p:n:s:")) != -1)
        switch (c)
        {
        case 'p':
            perms = strtol(optarg, (char **)NULL, 8); // optarg to np. "0644", 8 mowi, ze czytamy to jako system osemkowy
            break;
        case 's':
            size = strtol(optarg, (char **)NULL, 10);
            break;
        case 'n':
            name = optarg;
            break;
        case '?':
        default: // w kazdej innej opcji blad
            usage(argv[0]);
        }
    if ((NULL == name) || ((mode_t)-1 == perms) || (-1 == size)) // jesli petla sie skonczyla, a uzytkownik nie podal jakiejs flagi
        usage(argv[0]);
    if (unlink(name) && errno != ENOENT) // jesli pliku nie ma to spoko, jesli jest to cos innego poszlo nie tak i ubijamy
        ERR("unlink");
    srand(time(NULL)); // za kazdym razem inny rezultat, ustawiamy ziarno raz, NAJLEPIEJ NA POCZATKU PROGRAMU
    make_file(name, size, perms, 10);
    return EXIT_SUCCESS;
}