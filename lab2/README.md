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

`filedes` - deskryptor pliku, pozyskany z open,   
`buf` - wskaźnik na bufor z którego pobierane będą dane,   
`nbyte` - rozmiar danych przeznaczonych do wysłania.

**Zwracana wartość:** ilość zapisanych bajtów.

### man 3p lseek

```C
#include <unistd.h>

off_t lseek(int fildes, off_t offset, int whence);
```

Jako parametr whence możemy podać `SEEK_SET` (ustawienie kursora na daną pozycję), `SEEK_CUR` (przesunięcie kursora względem aktualnej pozycji) albo `SEEK_END` (przesunięcie kursora za koniec pliku). Jako, że lseek zwraca pozycję kursora po wykonaniu operacji, wywołanie go z parametrami `0` i `SEEK_CUR` pozwala uzyskać aktualną pozycję w pliku.

### man 3p writev

```C
#include <sys/uio.h>

ssize_t writev(int fildes, const struct iovec *iov, int iovcnt);
```

Funkcja `writev` oferuje wygodne rozwiązanie w przypadku gdy dane które chcemy zapisać nie znajdują się w jednym ciągłym fragmencie pamięci. Pozwala ona zebrać dane z wielu miejsc i zapisać je w pliku za pomocą jednego wywołania funkcji. Znajduje się w nagłówku `<sys/uio.h>`.

`filedes` - deskryptor do którego zapisywane są dane,   
`iov` - tablica struktur opisujących bufory z których funkcja zbiera dane - struct iovec (man 0p sys_uio.h), które mają następujące pola:   
`void   *iov_base` -> wskaźnik na obszar pamięci,   
`size_t  iov_len`  -> długość obszaru pamięci,   
`iovcnt` - rozmiar tablicy `iov`.   
Funkcja ta zapisuje do filedes:   
`iov[0].iov_len` bajtów zaczynając od `iov[0].iov_base`, następnie   
`iov[1].iov_len` bajtów zaczynając od `iov[1].iov_base`, … aż do   
`iov[iovcnt-1].iov_len` bajtów zaczynając od `iov[iovcnt-1].iov_base`.

```C
ssize_t bytes_written;
int fd;
char *buf0 = "short string\n";
char *buf1 = "This is a longer string\n";
char *buf2 = "This is the longest string in this example\n";
int iovcnt;
struct iovec iov[3];

iov[0].iov_base = buf0;
iov[0].iov_len = strlen(buf0);
iov[1].iov_base = buf1;
iov[1].iov_len = strlen(buf1);
iov[2].iov_base = buf2;
iov[2].iov_len = strlen(buf2);
...
iovcnt = sizeof(iov) / sizeof(struct iovec); // WIelkosc w bajtach calej tablicy na rozmiar jednego elementu

bytes_written = writev(fd, iov, iovcnt); // Suma dlugosci wszystkich trzech lancuchow
```

### man 3p readv

```C
#include <sys/uio.h>

ssize_t readv(int fildes, const struct iovec *iov, int iovcnt);
```

`filedes` - deskryptor z którego czytane są dane,   
`iov` - tablica struktur struct iovec opisujących bufory do których funkcja rozprasza dane,   
`iovcnt` - rozmiar tablicy `iov`.

```C
ssize_t bytes_read;
int fd;
char buf0[20];
char buf1[30];
char buf2[40];
int iovcnt;
struct iovec iov[3];

iov[0].iov_base = buf0;
iov[0].iov_len = sizeof(buf0);
iov[1].iov_base = buf1;
iov[1].iov_len = sizeof(buf1);
iov[2].iov_base = buf2;
iov[2].iov_len = sizeof(buf2);
...
iovcnt = sizeof(iov) / sizeof(struct iovec);

bytes_read = readv(fd, iov, iovcnt);
```


### man 3p mmap

```C
#include <sys/mman.h>

void *mmap(void *addr, size_t len, int prot, int flags,
int fildes, off_t off);
```

**Jak działa?**

**Połączenie:** Ustanawia bezpośrednie mapowanie między przestrzenią adresową Twojego programu (pamięcią RAM) a plikiem na dysku.   
**Wskaźnik:** Zwraca zwykły wskaźnik (adres) do tej pamięci.   
**Magia w tle:** Traktujesz plik dokładnie tak samo jak lokalną tablicę w C (np. `buff[0] = 'X'`). Nie używasz read ani write, a system operacyjny automatycznie, w tle sprzęga te modyfikacje w RAM-ie z fizycznym dyskiem twardym.

**Argumenty funkcji**

* **`addr` (Sugerowany adres):** Określa miejsce w przestrzeni adresowej twojego procesu, w pobliżu którego system powinien umieścić mapowanie. W praktyce wpisuje się tu najczęściej `0` (czyli `NULL`), co daje systemowi całkowitą swobodę w bezpiecznym dobraniu adresu.
* **`len` (Długość):** Dokładna liczba bajtów, dla których chcesz ustanowić mapowanie. Jeśli wpiszesz tu zero, funkcja natychmiast zwróci błąd i mapowanie nie powstanie.
* **`prot` (Ochrona/Uprawnienia):** Zestaw flag określających, na jakie operacje w tej pamięci pozwala twój kod. Używasz tu wartości takich jak `PROT_READ` (dozwolony odczyt danych), `PROT_WRITE` (dozwolony zapis), `PROT_EXEC` (wykonywanie kodu) lub `PROT_NONE` (całkowity brak dostępu), które możesz łączyć operatorem logicznym OR (`|`).
* **`flags` (Flagi zachowania):** Decyduje o tym, co dzieje się z danymi po ich modyfikacji. Musisz wybrać jedną z dwóch głównych opcji:
* `MAP_SHARED`: Wszelkie zmiany wprowadzone w pamięci wpłyną na bazowy obiekt (czyli fizycznie zaktualizują plik na dysku).
* `MAP_PRIVATE`: Modyfikacje pozostają całkowicie prywatne dla twojego procesu i nie zmieniają oryginalnego pliku na dysku.
* **`fildes` (Deskryptor pliku):** Liczba całkowita reprezentująca otwarty plik, z którym chcesz powiązać pamięć (uzyskana wcześniej z funkcji `open`).
* **`off` (Przesunięcie/Offset):** Wskazuje, od którego bajtu w pliku źródłowym ma rozpocząć się mapowanie. Wartość ta najczęściej musi być wielokrotnością rozmiaru strony pamięci (zwykle 4096 bajtów), zwracanego przez systemową funkcję `sysconf()`. Jeśli chcesz zmapować plik od samego początku, wpisujesz tu `0`.

**Wartość zwracana**

* **W przypadku sukcesu:** Wskaźnik (adres) do miejsca w pamięci operacyjnej, pod którym zmapowano plik.
* **W przypadku błędu:** Specjalną wartość `MAP_FAILED` oraz ustawia systemową zmienną `errno` wskazującą przyczynę awarii.

**Przykład**

`char *buff = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);`   
W ten sposób prosimy system operacyjny o "zmapowanie" pliku o deskryptorze `fd` do pamięci o rozmiarze `length` tak, żebyśmy mogli go zarówno czytać jak i modyfikować (`PROT_READ | PROT_WRITE`). Wynikiem jest wskaźnik na zmapowaną pamięć lub wartość `MAP_FAILED` gdy wystąpi błąd.   
`mmap` pozwala nam bardzo wygodnie modyfikować pliki, tak samo jak zwykłe tablice. Dwie funkcje pomocnicze to `msync` i `munmap`.

`msync` wymusza synchronizację zmian na dysk - system operacyjny nie robi tego zawsze od razu, ponieważ zapisy na dysk są wolne.   
`munmap` to likwidacja mapowania, czyli sprzątanie.

**UWAGA!!! Deskryptor musi mieć odpowiednie uprawnienia. Gdybyśmy do `mmap` podali `PROT_READ | PROT_WRITE` oraz deskryptor który otworzyliśmy np. z flagą `O_RDONLY` funkcja zwróciłaby błąd.**

```C
int fd = open("a.txt", O_RDWR);
if (fd == -1)
  ERR("open");

struct stat sb;
if (fstat(fd, &sb) == -1)
  ERR("fstat");
size_t length = sb.st_size;
char *buff = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if (buff == MAP_FAILED)
  ERR("mmap");

printf("a.txt[3] = %c (ASCII: %d)\n", buff[3], buff[3]); // To sie dzieje OD RAZU
buff[3] = 'Z'; // Modyfikuje tekst w moim RAMie
msync(buff, length, MS_SYNC); // Wymusza zapisanie modyfikacji na dysk. Flaga MS_SYNC oznacza, że program poczeka w tej linijce, aż dysk twardy potwierdzi udany zapis. Gdybyś użyła MS_ASYNC, system przyjąłby zlecenie, ale program od razu poszedłby dalej.

munmap(buff, length); // Zrywa mapowanie, oddaje RAM i buf[0] grozi od teraz segmentation fault
close(fd);
```

### man 3p fcntl

```C
#include <fcntl.h>

int fcntl(int fildes, int cmd, ...);
```

Podanie `F_GETFL` sprawia, że funkcja zwraca aktualnie ustawione flagi jako maskę bitową zapisaną w zmiennej typu `int`.
Użycie `F_SETFL` wymaga podania maski definiującej flagi, jakie chcemy ustawić. Dlatego zwykle proces ustawiania flag deskryptora wygląda jak w funkcji `set_nonblocking()` w poniższym kodzie. Najpierw pobieramy aktualne flagi, ustawiamy te pola, które nas interesują, a następnie zapisujemy. Próba ustawienia flag poprzez wywołanie np. `fcntl(fd, F_SETFL, O_NONBLOCK)` jest błędem, ponieważ w ten sposób przy okazji **resetujemy wszystko** poza flagą `O_NONBLOCK`.

```C
void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL); // Pobiera wszystkie obecne ustawienia tego pliku
    if (flags == -1)
        ERR("fcntl");
    flags |= O_NONBLOCK;                 // Dodaje flage do ustawien
    if (fcntl(fd, F_SETFL, flags) == -1) // SETFL - set file status flags
        ERR("fcntl");
}
```

### man 3p poll

```C
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

`fds` - tablica struktur z desktryptorami, na które będziemy czekać,   
`nfds` - oczywiście rozmiar tej tablicy,   
`timeout` - poll zakończy się gdy któryś z deskryptorów będzie gotowy do wykonania operacji albo osiągnięty zostanie `timeout` w milisekundach. Możemy też podać tam wartość `-1` żeby czekać w nieskończoność.

**Działanie funkcji**

Użycie funkcji `poll` polega więc na stworzeniu tablicy `fds` i wpisanie tam deskryptorów oraz wydarzeń na które będziemy czekać. Na przykład chcąc czekać aż jakieś użytkownik wpisze jakieś dane na standardowe wejście ustawimy `.fd` na `0` lub `STDIN_FILENO` a `.events` na `POLLIN`. **`revents` będzie ustawiany przez samego poll-a, dla bezpieczeństwa najlepiej go wyzerować.**

**Wartości zwracane przez funkcję**

* W przypadku pomyślnego zakończenia wywołania, funkcja zwraca wartość nieujemną.
* Wartość dodatnia wskazuje całkowitą liczbę struktur `pollfd`, które zarejestrowały wybrane zdarzenia. Oznacza to liczbę struktur, w których pole `revents` ma wartość różną od zera.
* Wartość 0 oznacza, że upłynął określony czas oczekiwania (`timeout`). Wskazuje to również, że nie wybrano żadnych deskryptorów plików.
* W przypadku niepowodzenia funkcja zwraca wartość `-1`. W takiej sytuacji system ustawia zmienną `errno`, aby wskazać konkretny błąd.

**Flagi zdarzeń w polu `revents`**

* **POLLIN:** Dane inne niż te o wysokim priorytecie mogą być odczytane bez blokowania.
* **POLLRDNORM:** Normalne dane mogą być odczytane bez blokowania.
* **POLLRDBAND:** Dane priorytetowe mogą być odczytane bez blokowania.
* **POLLPRI:** Dane o wysokim priorytecie mogą być odczytane bez blokowania.
* **POLLOUT:** Normalne dane mogą być zapisane bez blokowania.
* **POLLWRNORM:** Flaga jest odpowiednikiem `POLLOUT`.
* **POLLWRBAND:** Można zapisywać dane priorytetowe.
* **POLLERR:** Na urządzeniu lub strumieniu wystąpił błąd. Ta flaga jest poprawna wyłącznie w masce `revents` i jest ignorowana w polu `events`.
* **POLLHUP:** Urządzenie zostało odłączone, albo potok lub FIFO zostały zamknięte przez ostatni proces, który miał je otwarte do zapisu. Ta flaga jest poprawna wyłącznie w masce `revents` i jest ignorowana w polu `events`.
* **POLLNVAL:** Podana wartość deskryptora `fd` jest nieprawidłowa. Ta flaga jest poprawna wyłącznie w polu `revents` i jest ignorowana w polu `events`.

Funkcja `poll()` automatycznie ustawi flagi `POLLERR`, `POLLHUP` oraz `POLLNVAL` w polu `revents`, jeśli dany warunek będzie prawdziwy, nawet w przypadku, gdy aplikacja nie ustawiła ich w polu `events`.

**UWAGA!!! Wartość dokumentacji wskazuje tu na jedną ważną zasadę: zdarzenia `POLLHUP` i `POLLOUT` wykluczają się wzajemnie.** Strumień nie może być jednocześnie rozłączony (`POLLHUP`) i gotowy na przyjmowanie nowych danych (`POLLOUT`).

`fds[i].fd < 0` oznacza, że urządzenie jest **martwe (odłączone).**

### man 0p poll.h

`int fd` - deskryptor na który będziemy czekać.   
`short events` - bitmaska w której zapisujemy wydarzenia (events) na które chcemy czekać. Dla na istotne jest przede wszystkim `POLLIN` czyli oczekiwanie aż **deskryptor będzie miał gotowe do odczytu dane,** oraz `POLLOUT` - gdy **deskryptor będzie gotowy do zapisu.**   
`short revents` - od return events - w tej bitmasce poll ustawi flagi wskazujące, co było **przyczyną jego zakończenia** - może to być przede wszystkim to, co ustawiliśmy w events (`POLLIN` lub `POLLOUT`), ale także wartości oznaczające, że wystąpił błąd lub problem z deskryptorem.
