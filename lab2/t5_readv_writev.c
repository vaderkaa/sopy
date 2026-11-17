#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef struct // struktura reprezentujaca ksiazke w pamieci
{
    unsigned int size; // 4 bajty
    char title[65];    // 64 bajty + 1 bajt na znak \0
} Book;

void usage(const char *pname)
{
    fprintf(stderr, "USAGE:%s database output\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 3) // plik wejsciowy i plik wyjsciowy
        usage(argv[0]);

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        ERR("open");
    struct stat s;
    if (fstat(fd, &s) < 0)
        ERR("fstat");
    const int size = s.st_size;
    const int N = size / 68; // obliczenie liczby rekordow

    // alokacja pamieci dla tablicy wektorow iovec
    struct iovec *buffers = malloc(N * sizeof(struct iovec)); // N wektorow typu struct iovec

    // alokacja pamieci dla tablicy struktur Book
    Book *books = malloc(N * sizeof(Book));
    if (buffers == NULL || books == NULL) // jesli cos sie zepsulo
        ERR("malloc");

    for (int i = 0; i < N; i++) // N wektorow bedziemy wpisywac
    {
        // ustawienie adresu bazowego i-tego wektora na poczatek pamieci i-tej ksiazki
        buffers[i].iov_base = &books[i];
        buffers[i].iov_len = 68; // ma zapisac 68 bajtow
        books[i].title[64] = 0;  // ustawienie \0, zeby mozna bylo potem wypisac tytul
    }
    // skopiowanie z fd danych i rozrzucenie ich po N wektorach
    if (readv(fd, buffers, N) < size) // jesli wczytano mniej niz calosc pliku to blad
        ERR("readv");
    if (close(fd))
        ERR("close");

    for (int i = 0; i < N; i++)
    {
        printf("Size: %u Title: %s\n", books[i].size, books[i].title);
        books[i].size++; // zwiekszenie rozmiaru ksiazki, bo tak kaza w poleceniu
    }

    // stworz jesli nie istnieje, a jesli istnieje to utnij do 0
    if ((fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
        ERR("open");
    if (writev(fd, buffers, N) < size) // zebranie danych z buforow (z kazdego 68 bajtow, ignoruje \0) do nowego pliku
        ERR("writev");
    if (close(fd))
        ERR("close");

    free(buffers);
    free(books);
    return EXIT_SUCCESS;
}