# Sopy 1

### man 3p fdopendir
```C
#include <dirent.h>
DIR *opendir(const char *dirname);
DIR *fdopendir(int fd);
```

### readdir
```C
struct dirent *readdir(DIR *dirp);
```

### man 0p dirent.h
```C
ino_t  d_ino       -> identyfikator pliku (numer inode, więcej w man 7 inode)
char   d_name[]    -> nazwa pliku
```

### man 3p fstat
```C
#include <sys/stat.h>
int  stat(const char *restrict path, struct stat *restrict buf);
int lstat(const char *restrict path, struct stat *restrict buf);
```

Jedyną różnicą w działaniu funkcji stat i lstat jest obsługa linków. stat zwróci informacje o pliku, do którego dany link prowadzi, natomiast lstat zwróci informacje o samym linku.

```C
struct stat buffer;
fstat(fildes, &buffer);
```

Kiedy piszesz `struct stat buffer;` system automatycznie tworzy dla Ciebie gotowe "pudełko" w pamięci, które pomieści wszystkie informacje o pliku.   
Jednak funkcja `fstat` oczekuje wskaźnika (adresu do pudełka), by móc do niego zajrzeć i je wypełnić. Dlatego dodajemy `&` przed `buffer`. Instrukcja `&buffer` mówi: "weź to moje pudełko i zrób mi wskaźnik na jego adres".

```C
struct stat *buffer;
fstat(fildes, buffer);								// Tu nie trzeba &, bo buffer to już wskaźnik
```

Ten kod się skompiluje, ale program natychmiast ulegnie awarii (tzw. Segmentation Fault). Dlaczego?   
Ponieważ instrukcja `struct stat *buffer;` tworzy tylko wskaźnik (wizytówkę z adresem), ale nie tworzy samego pudełka na dane. Taki wskaźnik pokazuje w bliżej nieokreślone, losowe miejsce w pamięci (tzw. dziki wskaźnik). Kiedy fstat spróbuje wpisać tam dane, zepsuje pamięć programu i system operacyjny go brutalnie zabije.   
Aby to drugie podejście zadziałało, musiałbyś ręcznie zbudować "pudełko" za pomocą dynamicznej alokacji pamięci:

```C
struct stat *buffer = malloc(sizeof(struct stat)); // Ręczne rezerwowanie pamięci
fstat(fildes, buffer);                             // Podajemy czysty wskaźnik
// ... robimy coś z danymi ...
free(buffer);                                      // Musimy pamiętać o posprzątaniu!
```

### xargs - man 1 xargs

`string` to w pamięci po prostu znaki ułożone jeden po drugim, zakończone niewidocznym znakiem `\0`.   
`char *` to wskaźnik na pierwszy znak takiego ciągu. Mając ten adres, program wie, gdzie zaczyna się słowo i czyta je aż do napotkania `\0`.   Właśnie to traktujemy w C jako "string".   
Tablica wskaźników to zbiór adresów początkowych poszczególnych słów.   
`char** argv` to wskaźnik pokazujący, gdzie w pamięci zaczyna się ta cała tablica z adresami.   
`argv[i]` odwołuje się do i-tego parametru wywołania programu.   
Argument zerowy `argv[0]` to zawsze nazwa programu.   
Aby parametrem uczynić dwa lub więcej słów trzeba ująć je w apostrofy albo wpisać znak \ przed każdą spacją (lub innym tzw. białym znakiem).

### scandir

Rozbijmy ten potrójny wskaźnik (`struct dirent ***`) na części logiczne, krok po kroku:   
`struct dirent` – struktura opisująca pojedynczy plik lub katalog.   
`struct dirent *` (1 gwiazdka) – wskaźnik na ten konkretny plik.   
`struct dirent **` (2 gwiazdki) – tablica wskaźników na pliki (czyli cała lista plików znajdujących się w katalogu).   
`struct dirent ***` (3 gwiazdki) – adres, pod którym znajduje się ta tablica wskaźników.

# Zadanie z lab1

# C
```C
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(int argc, char* argv[])
{
    printf("%s pattern\n", argv[0]);
    printf("pattern - string pattern to search at standard input\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    char pattern = *argv[1];

    char* line = NULL;
    size_t line_len = 0;
    while (getline(&line, &line_len, stdin) != -1)  // man 3p getdelim
    {
        if (strstr(line, &pattern))
            printf("%s",line);  // getline() should return null terminated data
    }

    if(line)
        free(line);

    return EXIT_SUCCESS;
}
```

# Makefile
```Makefile
override CFLAGS=-Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-const-variable -g -O0 -fsanitize=address,undefined

ifdef CI
override CFLAGS=-Wall -Wextra -Wshadow -Werror -Wno-unused-parameter -Wno-unused-const-variable
endif

NAME=sop-grep

.PHONY: clean all

all: ${NAME}

${NAME}: ${NAME}.c
	gcc $(CFLAGS) -o ${NAME} ${NAME}.c

clean:
	rm -f ${NAME}
```

# Polecenie
1. 0 p. Skompiluj program i sprawdź, w jakim jest stanie. Napraw wszystkie ostrzeżenia i błędy
kompilatora.
2. 0 p. Rozwiąż wszystkie znalezione problemy i spraw, aby program działał zgodnie z treścią zadania.
3. 0 p. Dodaj do programu obsługę zmiennej środowiskowej W1_LINENUMBER. Przypisanie jej wartości
1, TRUE albo true powoduje, że program wypisuje najpierw numer linii (wejścia), w której wystąpił
wzorzec a resztę linii po dwukropku. Przykład:
5:linia z szukanym wzorcem.
4. 1 p. Dodaj do programu obsługę zmiennej środowiskowej W1_LOGFILE. Jeśli ma ona przypisaną
wartość program zapisuje swoje wyjście również do pliku o ścieżce będącej tą wartością. Jeśli plik
nie istnieje należy go utworzyć.
