# sopy

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

makefile
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

  
