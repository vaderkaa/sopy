# Kod startowy

```C
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname) 
{
    fprintf(stderr, "USAGE: %s\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) 
{
    // Tutaj kod programu
    
    return EXIT_SUCCESS;
}
```

# Makefile

```Makefile
CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = zadanie

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)
```

# Stringi

### Długość stringów

`strlen(const char *s)`   
**Parametry:** `s` – wskaźnik na string wejściowy.   
**Działanie:** Liczy znaki w stringu aż do napotkania pierwszego bajtu zerowego.   
**Zwraca:** `size_t` – liczba znaków (nie wliczając zera na końcu).   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy (tylko czyta).   

`strnlen(const char *s, size_t maxlen)` *(POSIX)*   
**Parametry:** `s` – wskaźnik na string; `maxlen` – maksymalna liczba znaków do sprawdzenia.   
**Działanie:** Liczy znaki, ale zatrzymuje się po sprawdzeniu `maxlen` bajtów (chroni przed brakiem `\0` i wyjściem poza pamięć).   
**Zwraca:** `size_t` – długość stringa, lub dokładnie `maxlen`, jeśli nie znaleziono zera.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy.   

### Kopiowanie

`strcpy(char *dest, const char *src)`   
**Parametry:** `dest` – bufor docelowy; `src` – string kopiowany.   
**Działanie:** Kopiuje całą zawartość `src` do `dest`. Bufor `dest` musi być odpowiednio duży.   
**Zwraca:** `char*` – wskaźnik na `dest`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Tak, kopiuje go z `src` na sam koniec.   

`strncpy(char *dest, const char *src, size_t n)`   
**Parametry:** `dest` – bufor docelowy; `src` – string kopiowany; `n` – maksymalna liczba bajtów do zapisania.   
**Działanie:** Kopiuje maksymalnie `n` bajtów z `src` do `dest`.   
**Zwraca:** `char*` – wskaźnik na `dest`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** UWAGA: Jeśli `src` ma mniej znaków niż `n`, reszta bufora `dest` (aż do `n`) jest wypełniana zerami (`\0`). Jeśli `src` ma tyle samo lub więcej znaków niż `n`, funkcja NIE DOKLEJA znaku `\0`. Wynikowy string nie będzie zakończony zerem! Musisz zrobić to ręcznie: `dest[n-1] = '\0';`.   

`strdup(const char *s)` *(POSIX)*   
**Parametry:** `s` – string do zduplikowania.   
**Działanie:** Mierzy długość `s`, prosi system o pamięć, kopiuje tam `s`.   
**Zwraca:** `char*` – wskaźnik do nowej, wyizolowanej kopii stringa. Zwraca `NULL` w przypadku błędu.   
**Alokacja pamięci:** TAK. Używa `malloc` pod maską. Zwrócony wskaźnik musisz kiedyś zwolnić przez `free()`.   
**Znak `\0`:** Tak, pamięć jest alokowana z zapasem na `\0` i znak jest kopiowany.   

`strndup(const char *s, size_t n)` *(POSIX)*   
**Parametry:** `s` – string do zduplikowania; `n` – maksymalna liczba znaków do skopiowania.   
**Działanie:** Działa jak `strdup`, ale kopiuje najwyżej `n` znaków.   
**Zwraca:** `char*` – wskaźnik do nowej kopii. Zwraca `NULL` przy błędzie.   
**Alokacja pamięci:** TAK. Trzeba zrobić `free()`.   
**Znak `\0`:** Tak, zawsze dokleja `\0` na końcu, niezależnie od długości pobranej z `src`.   

### Łączenie (Konkatenacja)

`strcat(char *dest, const char *src)`   
**Parametry:** `dest` – string bazowy, do którego doklejamy; `src` – string doklejany.   
**Działanie:** Szuka zera kończącego w `dest` i w to miejsce zaczyna nadpisywać znaki z `src`. Bufor `dest` musi mieć odpowiedni zapas pamięci.   
**Zwraca:** `char*` – wskaźnik na `dest`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Tak, dokleja go zawsze na samym końcu nowego stringa.   

`strncat(char *dest, const char *src, size_t n)`   
**Parametry:** `dest` – bazowy string; `src` – doklejany string; `n` – maksymalna liczba znaków pobrana z `src`.   
**Działanie:** Bezpieczniejsza wersja `strcat`. Pobiera najwyżej `n` znaków.   
**Zwraca:** `char*` – wskaźnik na `dest`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Tak. W przeciwieństwie do `strncpy`, funkcja `strncat` zawsze dokleja jeden ostateczny `\0`. Zapisze więc maksymalnie `n + 1` bajtów.   

### Porównywanie

`strcmp(const char *s1, const char *s2)`   
**Parametry:** `s1`, `s2` – stringi do porównania.   
**Działanie:** Porównuje kody ASCII znak po znaku aż znajdzie różnicę lub dojdzie do zera. Wrzuca znaki na szale wagi.   
**Zwraca:** `int`. Zwraca `0`, jeśli są identyczne. Wartość ujemną (`< 0`), jeśli w pierwszym różniącym się miejscu `s1` ma mniejszy kod ASCII niż `s2`. Wartość dodatnią (`> 0`), jeśli większy.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy.   

`strncmp(const char *s1, const char *s2, size_t n)`   
**Parametry:** `s1`, `s2` – stringi; `n` – ile pierwszych znaków porównać.   
**Działanie:** Jak `strcmp`, ale zatrzymuje się po sprawdzeniu `n` znaków.   
**Zwraca:** `int`. Zwraca `0` jeśli pierwsze `n` znaków jest identyczne. Inaczej wartość `< 0` lub `> 0`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy.   

`strcasecmp(const char *s1, const char *s2)` *(POSIX)*   
**Parametry:** `s1`, `s2` – stringi do porównania.   
**Działanie:** Jak `strcmp`, ale ignoruje wielkość liter (traktuje 'a' i 'A' jako to samo).   
**Zwraca:** `int` (`0`, ujemna, dodatnia).   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy.   

### Wyszukiwanie i parsowanie

`strchr(const char *s, int c)`   
**Parametry:** `s` – przeszukiwany string; `c` – szukany znak (kod ASCII jako `int`).   
**Działanie:** Szuka pierwszego wystąpienia znaku `c` od lewej strony.   
**Zwraca:** `char*` – wskaźnik na to fizyczne miejsce wewnątrz `s`, gdzie leży znaleziony znak. Jeśli nie znajdzie, zwraca `NULL`.   
**Alokacja pamięci:** Nie (zwraca wskaźnik do już istniejącej pamięci).   
**Znak `\0`:** Nie dotyczy, ale znak `\0` sam w sobie może być wyszukiwany.   

`strrchr(const char *s, int c)`   
**Parametry:** `s` – przeszukiwany string; `c` – szukany znak.   
**Działanie:** Szuka wystąpienia znaku `c` od prawej strony (ostatnie wystąpienie w stringu, np. do znalezienia rozszerzenia po ostatniej kropce).   
**Zwraca:** `char*` – wskaźnik na znak lub `NULL`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy.   

`strstr(const char *haystack, const char *needle)`   
**Parametry:** `haystack` – "stóg siana", przeszukiwany string; `needle` – "igła", szukany pod-string (słowo).   
**Działanie:** Szuka pierwszego wystąpienia całego słowa `needle` wewnątrz `haystack`.   
**Zwraca:** `char*` – wskaźnik na początek znalezionego słowa wewnątrz `haystack` lub `NULL`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Nie dotyczy.   

`strtok(char *str, const char *delim)`   
**Parametry:** `str` – string do pocięcia w pierwszym wywołaniu, w kolejnych podajesz tu `NULL`; `delim` – zbiór znaków służących jako przecinaki (np. `" ,.-"`).   
**Działanie:** Dzieli string na "tokeny" (kawałki). Wywołana pierwszy raz na stringu znajduje pierwszy token. Przy każdym kolejnym wywołaniu z parametrem `str = NULL`, szuka kolejnego w ukrytej zmiennej wewnątrz biblioteki.   
**Zwraca:** `char*` – wskaźnik na początek znalezionego tokena. Kiedy nie ma już więcej słów, zwraca `NULL`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** TAK (destrukcja). Funkcja niszczy Twój oryginalny string, fizycznie podmieniając znalezienie przecinaki (`delim`) na znaki końca `\0`, po to, żeby zwrócony wskaźnik działał jako osobny string.   
**Uwaga:** Nie jest bezpieczna w wielowątkowości (tzw. thread-unsafe), bo używa ukrytych zmiennych statycznych.   

`strtok_r(char *str, const char *delim, char **saveptr)` *(POSIX)*   
**Parametry:** `str` i `delim` jak wyżej; `saveptr` – adres wskaźnika, w którym funkcja sama będzie zapisywać swój stan między wywołaniami.   
**Działanie:** To samo co `strtok`, ale bezpieczne (reentrant). Ty dajesz jej miejsce na notatnik (`saveptr`), więc niczego nie chowa pod spodem.   
**Zwraca:** `char*` na token lub `NULL`.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Tak, tak samo niszczy (nadpisuje) oryginalny string zerami.   

`strspn(const char *s, const char *accept)`   
**Parametry:** `s` – badany string; `accept` – lista akceptowanych znaków.   
**Działanie:** Idzie od lewej po `s` i mierzy, ile znaków pod rząd należy do zbioru `accept`. Zatrzymuje się na pierwszym znaku niebędącym w tym zbiorze.   
**Zwraca:** `size_t` – długość początkowego fragmentu stringa składającego się tylko z dozwolonych liter.   
**Alokacja pamięci:** Nie.   

`strcspn(const char *s, const char *reject)`   
**Parametry:** `s` – badany string; `reject` – lista zabronionych znaków.   
**Działanie:** Odwrotność `strspn`. Mierzy, ile znaków pod rząd nie należy do `reject`. Przydaje się np. do sprawdzania "jak daleko jest do pierwszej spacji lub nowej linii".   
**Zwraca:** `size_t` – liczba znaków do pierwszego uderzenia w "zabroniony" znak.   
**Alokacja pamięci:** Nie.   

### Pisanie formatowane bezpośrednio do zmiennej (`<stdio.h>`)

`sprintf(char *str, const char *format, ...)`   
**Parametry:** `str` – bufor docelowy; `format` – string sterujący z `%s`, `%d` itd.; `...` – zmienne do wklejenia.   
**Działanie:** Działa jak `printf`, ale zamiast wyrzucać tekst na ekran (konsolę), drukuje go do zmiennej `str`.   
**Zwraca:** `int` – liczba faktycznie wpisanych znaków (bez końcowego zera).   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Tak, automatycznie zabezpiecza wynik zerem na końcu. Jeśli wpiszesz za dużo danych, nadpiszesz pamięć obok (niebezpieczeństwo tzw. Buffer Overflow).   

`snprintf(char *str, size_t size, const char *format, ...)`   
**Parametry:** `str` – bufor; `size` – fizyczny rozmiar bufora (maksymalna pojemność); `format` i `...` jak wyżej.   
**Działanie:** Dokładnie to samo, ale ma nałożony twardy limit zapisywanych bajtów, dzięki czemu nigdy nie wyjedziesz poza pamięć.   
**Zwraca:** `int` – liczba znaków, która powinna zostać zapisana, gdyby było dość miejsca. Jeśli zwraca `>= size`, wiesz, że string został obcięty.   
**Alokacja pamięci:** Nie.   
**Znak `\0`:** Tak, ZAWSZE dokleja `\0` na końcu. Wykorzysta maksymalnie `size - 1` bajtów na znaki, a ostatni zostawi na `\0`. Posiada najwyższy certyfikat kuloodporności.   

# Operacje na pamięci

🚨 Funkcje `mem...` są GŁUCHE I ŚLEPE na znak końca stringa (`\0`). 🚨   
One nie operują na tekście, tylko na czystych blokach pamięci (bajtach). Znak `\0` nic dla nich nie znaczy (jest to dla nich po prostu bajt o wartości `0x00`). Zawsze musisz im dokładnie powiedzieć w parametrze `n`, ile bajtów mają przetworzyć. Gdy zaczną kopiować lub czytać, nie zatrzymają się po napotkaniu `\0`. Zrobią równo tyle kroków, ile im kazano, nawet jeśli wyjdą poza sensowne dane i zaczną czytać śmieci.

### Kopiowanie pamięci

`memcpy(void *dest, const void *src, size_t n)` *(Memory Copy)*   
**Parametry:** `dest` – wskaźnik na miejsce docelowe; `src` – wskaźnik na źródło, skąd bierzemy dane; `n` – dokładnie ile bajtów przekopiować.   
**Działanie:** Kopiuje twardo `n` bajtów z `src` do `dest`. Krytyczna uwaga: `dest` i `src` nie mogą na siebie nachodzić w pamięci! Jeśli nachodzą (tzw. overlapping), to zachowanie jest niezdefiniowane (może zepsuć dane).   
**Zwraca:** `void*` – wskaźnik na `dest` (niezmieniony początek docelowego bufora).   
**Alokacja pamięci:** NIE. Bufor `dest` musi być wcześniej zaalokowany przez Ciebie.   
**Znak \0:** NIE. Nigdy go nie dokleja. Po prostu przekleja `n` bajtów i kończy pracę.   

`memmove(void *dest, const void *src, size_t n)` *(Memory Move)*   
**Parametry:** `dest`, `src`, `n` – dokładnie to samo co w `memcpy`.   
**Działanie:** Kopiuje `n` bajtów z `src` do `dest`, ale JEST BEZPIECZNA dla nachodzących na siebie pamięci. Zanim skopiuje, zachowuje się tak, jakby najpierw skopiowała dane do tymczasowego, bezpiecznego bufora (dzięki odpowiedniej kolejności kopiowania od przodu lub od tyłu). Używaj jej, gdy np. przesuwasz kawałek tablicy wewnątrz tej samej tablicy.   
**Zwraca:** `void*` – wskaźnik na `dest`.   
**Alokacja pamięci:** NIE (pod spodem też nie alokuje z użyciem `malloc`, po prostu sprytnie przesuwa wskaźniki).   
**Znak \0:** NIE.   

`memccpy(void *dest, const void *src, int c, size_t n)` *(POSIX - Memory Copy until Character)*   
**Parametry:** `dest` i `src` jak wyżej; `c` – bajt-wyzwalacz (znak stopu); `n` – twardy limit bajtów.   
**Działanie:** Kopiuje bajty z `src` do `dest`, ale zatrzymuje się w jednym z dwóch przypadków: albo skopiuje ustalony bajt `c` (wtedy kończy natychmiast), albo dobije do limitu `n` bajtów.   
**Zwraca:** `void*` – jeśli znalazła bajt `c`, zwraca wskaźnik na miejsce w `dest` zaraz za tym bajtem. Jeśli nie znalazła `c` i po prostu wyczerpała `n`, zwraca `NULL`.   
**Alokacja pamięci:** NIE.   
**Znak \0:** NIE. Nie dokleja go automatycznie na końcu (chyba że to szukany znak `c` i został skopiowany ze źródła).   

### Wypełnianie i czyszczenie pamięci

`memset(void *s, int c, size_t n)` *(Memory Set)*   
**Parametry:** `s` – wskaźnik na początek bloku pamięci; `c` – wartość (zapisana jako `int`, ale funkcja obcina ją do rozmiaru jednego bajta, czyli `unsigned char`); `n` – ile kolejnych bajtów nadpisać.   
**Działanie:** Zamienia (wypełnia) każdy z `n` bajtów zaczynając od adresu `s` na bajt o wartości `c`. Używa się jej masowo do zerowania nowo utworzonych struktur (`memset(&moja_struktura, 0, sizeof(moja_struktura));`).   
**Zwraca:** `void*` – wskaźnik na `s` (niezmieniony).   
**Alokacja pamięci:** NIE. Nadpisuje istniejącą pamięć.   
**Znak \0:** NIE. Chyba że wpiszesz tam wartość `0`, no to po prostu wypełni pamięć zerami (czyli znakami `\0`).   

### Porównywanie

`memcmp(const void *s1, const void *s2, size_t n)` *(Memory Compare)*   
**Parametry:** `s1` i `s2` – wskaźniki na dwa bloki pamięci; `n` – ile bajtów porównać.   
**Działanie:** Krok po kroku, bajt po bajcie porównuje oba bloki. Nie zatrzymuje się na bajcie zerowym! Zawsze przetworzy równo `n` bajtów (chyba że wcześniej znajdzie różnicę). Traktuje te bajty jako wartości liczbowe (`unsigned char`).   
**Zwraca:** `int`. Zwraca `0`, gdy całe `n` bajtów jest identyczne. Zwraca wartość `< 0` lub `> 0`, jeśli na pierwszym różniącym się bajcie w `s1` jest wartość mniejsza/większa niż w `s2`.   
**Alokacja pamięci:** NIE.   
**Znak \0:** Zignorowany. Porówna go jak każdą inną liczbę i pójdzie sprawdzając dalsze bajty (jeśli `n` jeszcze na to pozwala).   

### Wyszukiwanie

`memchr(const void *s, int c, size_t n)` *(Memory Character)*   
**Parametry:** `s` – przeszukiwany blok; `c` – szukany bajt (znak); `n` – w jakiej puli bajtów szukać.   
**Działanie:** Przegląda `n` pierwszych bajtów bloku `s` od lewej strony, szukając pierwszego wystąpienia bajtu o wartości `c`. Odpowiednik `strchr`, ale działa na binarnych plikach/strukturach i ma twardy limit.   
**Zwraca:** `void*` – wskaźnik na fizyczny adres pierwszego trafienia w bloku `s`. Zwraca `NULL`, jeśli w ciągu `n` bajtów nie było tej wartości.   
**Alokacja pamięci:** NIE.   
**Znak \0:** Zignorowany, chyba że to on jest Twoim `c` (wtedy go znajdzie).   

`memrchr(const void *s, int c, size_t n)` *(GNU/Linux extension - Memory Reverse Character)*   
**Parametry:** Tak samo jak w `memchr`.   
**Działanie:** Dokładnie to samo co `memchr`, ale zaczyna szukać od prawej do lewej (od końca limitu `n` w stronę początku). Odpowiednik `strrchr`.   
**Zwraca:** `void*` (na ostatnie wystąpienie) lub `NULL`.   
**Alokacja pamięci:** NIE.   

`memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen)` *(GNU/Linux extension)*   
**Parametry:** `haystack` (stóg siana), `haystacklen` (długość stogu), `needle` (igła), `needlelen` (długość igły).   
**Działanie:** Szuka całego określonego kawałka pamięci (igły) wewnątrz większego bloku pamięci (stogu). Odpowiednik `strstr`, ale do danych binarnych (np. szukanie nagłówka JPEG w zrzuconym fragmencie RAM-u). Wymaga dokładnego podania długości obu bloków.   
**Zwraca:** `void*` – wskaźnik na początek igły w stogu lub `NULL`, jeśli jej nie ma.   
**Alokacja pamięci:** NIE.   
