# LAB 2

### man 3p open

```C
#include <sys/stat.h>
#include <fcntl.h>

int open(const char *path, int oflag, ...); // W 3 parametrze podajemy uprawnienia
int openat(int fd, const char *path, int oflag, ...);
```

Zgodnie z dokumentacją zawartą w pliku open.txt, wywołanie `open()` pozwala na użycie wielu flag konfiguracyjnych, z których należy wybrać dokładnie jedną podstawową flagę trybu dostępu, a następnie połączyć ją z dowolnymi flagami opcjonalnymi za pomocą bitowego OR (`|`).

**Podstawowe flagi trybu dostępu (należy wybrać dokładnie jedną):**

* `O_RDONLY`: Otwiera plik tylko do odczytu.
* `O_RDWR`: Otwiera plik do odczytu i zapisu. Zastosowanie tej flagi do pliku typu FIFO daje niezdefiniowany (undefined) rezultat.
* `O_WRONLY`: Otwiera plik tylko do zapisu.
* `O_EXEC`: Otwiera plik tylko do wykonywania (dotyczy plików niebędących katalogami). Rezultat jest nieokreślony (unspecified), jeśli użyje się tej flagi na katalogu.
* `O_SEARCH`: Otwiera katalog tylko w celu wyszukiwania. Rezultat jest nieokreślony, jeśli użyje się jej na pliku, który nie jest katalogiem.

**Ważne flagi dodatkowe (łączone przez `|`):**

* `O_CREAT`: Jeśli plik nie istnieje, zostanie utworzony jako zwykły plik (chyba że ustawiono też `O_DIRECTORY`). Jeśli plik już istnieje, flaga ta nie ma żadnego efektu, z wyjątkiem sytuacji użycia jej razem z `O_EXCL`.
* `O_EXCL`: Jeśli jest ustawiona razem z `O_CREAT`, funkcja `open()` zwróci błąd `[EEXIST]`, gdy plik już istnieje. Użycie `O_EXCL` bez `O_CREAT` daje niezdefiniowany rezultat.
* `O_APPEND`: Jeśli jest ustawiona, przed każdym zapisem przesunięcie w pliku (offset) zostanie ustawione na jego koniec (a normalnie jest na poczatku).
* `O_TRUNC`: Jeśli plik istnieje, jest zwykłym plikiem i został pomyślnie otwarty z flagą `O_RDWR` lub `O_WRONLY`, jego długość zostanie przycięta do 0 bajtów. Rezultat użycia tej flagi bez `O_RDWR` lub `O_WRONLY` jest niezdefiniowany.
* `O_NONBLOCK`: Otwiera plik w trybie nieblokującym. W przypadku plików FIFO lub specjalnych plików urządzeń blokowych/znakowych, funkcja zwraca wynik bez czekania na gotowość urządzenia.
* `O_DIRECTORY`: Jeśli podana ścieżka rozwiązuje się do pliku, który nie jest katalogiem, operacja kończy się błędem i ustawia zmienną `errno` na `[ENOTDIR]`.

**Co się dzieje przy połączeniu `O_EXEC` z `O_CREAT`?**

* Dokumentacja nie przypisuje temu połączeniu konkretnego kodu błędu, lecz stwierdza, że jeśli np. użyjemy `O_SEARCH | O_CREAT` (a nie wiadomo, czy `O_EXEC` i `O_SEARCH` mają pod spodem tę samą wartość) na nieistniejącym pliku, to nie jest określone (unspecified), czy zwykły plik w ogóle zostanie utworzony.
* Dodatkowo, jeśli zwrócony zostanie deskryptor pliku niebędącego katalogiem, jest nieokreślone, czy ten deskryptor będzie miał uprawnienia do wykonywania takie, jakich wymagałoby `O_EXEC`.

**Jakie błędy zwraca system przy dowiązaniach symbolicznych (symlinkach)?**

* Błąd `[EEXIST]`: Jeśli flagi `O_EXCL` i `O_CREAT` są ustawione, a ścieżka wskazuje na dowiązanie symboliczne, `open()` zawsze zwróci błąd `[EEXIST]`, niezależnie od tego, co zawiera ten symlink i czy wskazuje na istniejący plik.
* Błąd `[ELOOP]`: Jeśli ustawiono flagę `O_NOFOLLOW`, a ścieżka wskazuje na dowiązanie symboliczne, funkcja zawsze kończy się błędem `[ELOOP]`.
* Błąd `[ELOOP]`: Pojawia się również wtedy, gdy podczas rozwiązywania ścieżki system napotka na zapętlenie dowiązań symbolicznych lub przekroczy ich limit `{SYMLOOP_MAX}`.

**Jakie błędy zwraca system, jeśli docelowy zasób nie jest katalogiem (lub nim jest)?**

* Błąd `[ENOTDIR]`: Występuje, gdy ustawiono flagę `O_DIRECTORY`, a ścieżka wskazuje na plik niebędący katalogiem. Błąd ten wystąpi również, jeśli jakikolwiek element ścieżki jest już istniejącym plikiem, który nie jest katalogiem ani symlinkiem do katalogu.
* Błąd `[EISDIR]`: Pojawi się, gdy spróbujesz otworzyć istniejący katalog z flagami zapisu (`O_WRONLY` lub `O_RDWR`), albo jeśli użyjesz flagi `O_CREAT` w celu utworzenia pliku w miejscu, gdzie znajduje się katalog, ale nie podasz jednocześnie flagi `O_DIRECTORY`.

**Jakie inne błędy występują, gdy po prostu nie da się otworzyć pliku?**

* `[EACCES]`: Zwracane w przypadku braku uprawnień – np. gdy brakuje praw do przeszukiwania katalogu w ścieżce, gdy nie masz uprawnień odczytu/zapisu do pliku, lub brakuje praw zapisu w katalogu domierzanym przy próbie utworzenia pliku (`O_CREAT`) czy przycinania (`O_TRUNC`).
* `[ENOENT]`: Ścieżka wskazuje na pusty string, komponent ścieżki nie istnieje, lub plik nie istnieje a flaga `O_CREAT` nie została podana.
* `[EMFILE]`: Twój proces ma już otwarte wszystkie dostępne dla niego deskryptory plików.
* `[ENFILE]`: Osiągnięto globalny, systemowy limit maksymalnej liczby otwartych plików.

**Co mi daje `int openat(int fd, const char *path, int oflag, ...);`?**

Kiedy używasz zwykłej funkcji, np. `open("plik.txt", ...)`, system operacyjny zawsze szuka tego pliku w *bieżącym katalogu roboczym* (czyli w miejscu, z którego uruchomiłaś program). Jeśli plik znajduje się głęboko w innym miejscu na dysku, masz normalnie dwa wyjścia:

1. Sklejać w kodzie długie tekstowe ścieżki (np. `"/bardzo/dluga/sciezka/do/folderu/plik.txt"`), co jest niewygodne i pożera pamięć.
2. Zmienić katalog roboczy całego programu funkcją `chdir()`, co jest fatalnym pomysłem w programach wielowątkowych, ponieważ zmienia to lokalizację dla wszystkich wątków naraz.

Funkcje z sufiksem `-at` rozwiązują ten problem elegancko. Pozwalają podać dodatkowy argument – deskryptor otwartego wcześniej katalogu. Działa to tak:

1. Najpierw otwierasz sam docelowy folder, żeby uzyskać jego deskryptor (do tego służy flaga `O_SEARCH`, o którą pytałaś kilka wiadomości wcześniej). Zapisujesz go np. jako zmienną `dir_fd`.
2. Zamiast zwykłego `open()`, wywołujesz `openat(dir_fd, "plik.txt", ...)`.

W ten sposób dajesz systemowi jasny komunikat: "Zignoruj obecny katalog roboczy programu. Poszukaj pliku o nazwie `plik.txt`, traktując deskryptor `dir_fd` jako punkt startowy". Możesz używać samych krótkich nazw plików, niezależnie od tego, jak głęboko w strukturze dysku aktualnie operuje Twój kod.

Potem trzeba zamknąć otwarty folder!!!

### man 3p close

```C
#include <unistd.h>

int close(int fildes);
```

### man 3p read

```C
#include <unistd.h>

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset); // Pozycja startowa wewnątrz pliku
ssize_t read(int fildes, void *buf, size_t nbyte);
```

`filedes` - deskryptor pliku, pozyskany z open,   
`buf` - wskaźnik na bufor w którym zapisane będą dane,   
`nbyte` - rozmiar danych których ma oczekiwać funkcja.   

**1. Zwracane wartości i zachowanie wskaźnika:**

* **Zwraca to, co faktycznie przeczytał:** Funkcja zwraca liczbę rzeczywiście odczytanych bajtów, co może być mniejszą wartością, niż ta, o którą prosiłaś (np. jeśli plik się kończy przed wczytaniem całego bufora).
* **Koniec pliku (EOF) to zero:** Jeśli wskaźnik czytania znajduje się na końcu pliku (lub za nim) i spróbujesz czytać dalej, funkcja po prostu zwróci `0`.
* **Samo się przesuwa:** Po pomyślnym odczycie system automatycznie przesuwa wewnętrzny wskaźnik (offset) w pliku do przodu o tyle bajtów, ile udało się przed chwilą przeczytać. Dzięki temu wywołując `read()` w pętli, naturalnie idziesz przez plik do przodu.
* **Błąd to minus jeden:** Jeśli coś pójdzie nie tak (np. podasz nieprawidłowy deskryptor pliku), funkcja zwróci `-1` i odpowiednio ustawi systemową zmienną `errno`.

**2. Najczęstsze kody błędów (wartości zmiennej `errno`):**

* `[EBADF]`: Przekazany deskryptor nie jest prawidłowym deskryptorem pliku otwartym do odczytu.
* `[EISDIR]`: Deskryptor odnosi się do katalogu, a system nie pozwala na odczyt katalogów za pomocą funkcji `read()` (zamiast tego należy użyć wywołania `readdir()`).
* `[EINTR]`: Operacja odczytu została przerwana przez nadejście sygnału i nie przesłano żadnych danych.
* `[EIO]`: Wystąpił fizyczny błąd operacji wejścia/wyjścia (I/O) powiązanej z urządzeniem.
* `[ENOMEM]`: W systemie zabrakło pamięci do spełnienia żądania.
* `[ENOBUFS]`: Dostępnych jest zbyt mało zasobów w systemie, aby pomyślnie wykonać operację.
* `[EOVERFLOW]`: Plik jest zwykłym plikiem, a pozycja startowa odczytu przekroczyła lub zrównała się z systemowym maksimum dla offsetu pliku.

### man 3p write

```C
#include <unistd.h>

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
ssize_t read(int fildes, void *buf, size_t nbyte);
```

* `filedes` - deskryptor pliku, pozyskany z open.
* `buf` - wskaźnik na bufor z którego pobierane będą dane.
* `nbyte` - rozmiar danych przeznaczonych do wysłania.
* zwracana wartość: ilość zapisanych bajtów.

### man 3p lseek

```C
#include <unistd.h>

off_t lseek(int fildes, off_t offset, int whence);
```

Jako parametr whence możemy podać `SEEK_SET` (ustawienie kursora na daną pozycję), `SEEK_CUR` (przesunięcie kursora względem aktualnej pozycji) albo `SEEK_END` (przesunięcie kursora za koniec pliku). Jako, że lseek zwraca pozycję kursora po wykonaniu operacji, wywołanie go z parametrami `0` i `SEEK_CUR` pozwala uzyskać aktualną pozycję w pliku.
