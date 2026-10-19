# Sopy 1

### man 1 xargs

`string` to w pamięci po prostu znaki ułożone jeden po drugim, zakończone niewidocznym znakiem `\0`.   
`char *` to wskaźnik na pierwszy znak takiego ciągu. Mając ten adres, program wie, gdzie zaczyna się słowo i czyta je aż do napotkania `\0`.   Właśnie to traktujemy w C jako "string".   
Tablica wskaźników to zbiór adresów początkowych poszczególnych słów.   
`char** argv` to wskaźnik pokazujący, gdzie w pamięci zaczyna się ta cała tablica z adresami.   
`argv[i]` odwołuje się do i-tego parametru wywołania programu.   
Argument zerowy `argv[0]` to zawsze nazwa programu.   
Aby parametrem uczynić dwa lub więcej słów trzeba ująć je w apostrofy albo wpisać znak \ przed każdą spacją (lub innym tzw. białym znakiem).

### man 3p fprintf

`printf`: Wypisuje sformatowany tekst na standardowe wyjście (strumień `stdout`), czyli domyślnie na ekran terminala.   
`fprintf`: Wypisuje tekst do wskazanego strumienia, podanego jako argument `stream` (np. do pliku otwartego wcześniej za pomocą `fopen`).   
`dprintf`: Działa analogicznie do `fprintf`, ale zamiast wskaźnika na strumień, przyjmuje tzw. deskryptor pliku (`fildes`), czyli niskopoziomowy identyfikator pliku w postaci liczby całkowitej.   
`sprintf`: Zapisuje sformatowany tekst do bufora w pamięci (tablicy znaków `s`) i automatycznie dokleja na końcu znak terminacji `\0`. Użycie tej funkcji wymaga ostrożności, ponieważ to na programiście spoczywa obowiązek zagwarantowania, że w buforze jest wystarczająco dużo miejsca na cały tekst.   
`snprintf`: To bezpieczniejsza wersja funkcji `sprintf`, która przyjmuje dodatkowy argument `n` określający maksymalny rozmiar bufora. Funkcja ta upewnia się, że nie zapisze więcej niż `n` bajtów (nadmiarowe znaki są odrzucane), a na końcu zawsze umieszcza znak `\0`. Jeśli `n` wynosi zero, funkcja nic nie zapisuje do bufora.

`printf`, `fprintf` oraz `dprintf`: Zwracają całkowitą liczbę bajtów, która została pomyślnie wysłana do strumienia lub pliku.   
`sprintf`: Zwraca liczbę bajtów zapisanych do bufora `s`, nie wliczając w to końcowego znaku `\0`.   
`snprintf`: Zwraca liczbę bajtów, która zostałaby zapisana w buforze, gdyby limit `n` był wystarczająco duży (również z pominięciem znaku `\0`). Ta unikalna cecha pozwala programiście łatwo sprawdzić, czy jego bufor okazał się za mały, by pomieścić całą wygenerowaną treść.

Wpisanie danych `fprintf`, `fputc`, `fputs`, `fwrite` lub wczytanie ich `fscanf`, `fgetc`, `fgets`, `fread`.

### man 3p errno

```C
#include <errno.h>
```

### man 3p fdopendir

```C
#include <dirent.h>
DIR *opendir(const char *dirname);
DIR *fdopendir(int fd);
```

### man 3p closedir - TRZEBA O NIM PAMIĘTAĆ!!!

```C
int closedir(DIR *dirp);
```

### man 3p readdir

```C
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *restrict dirp, struct dirent *restrict entry, struct dirent **restrict result);
```

W celu przejrzenia całego katalogu, funkcję `readdir` należy wywoływać tyle razy, aż nie zwróci `NULL`. **W przypadku wystąpienia błędu, zarówno opendir, jak i readdir** zwracają `NULL`. Wynika z tego ważny wniosek w przypadku funkcji `readdir`: przed jej wywołaniem należy wyzerować zmienną `errno`, a w razie zwrócenia `NULL` sprawdzić, czy ta zmienna nie została ustawiona na niezerową wartość (oznaczającą błąd). errno jest zmienną globalną używaną przez funkcje systemowe do wskazania kodu napotkanego błędu.

Funkcje `stat`, `lstat` i `closedir` zwracają 0 w razie sukcesu, inna wartość oznacza błąd.

```C
DIR *katalog = opendir(".");
struct dirent *plik;

errno = 0; // Wyzerowanie przed pierwszym odczytem
while ((plik = readdir(katalog)) != NULL) {
    printf("%s\n", plik->d_name);
    errno = 0; // Wyzerowanie przed każdym kolejnym odczytem w pętli
}

// Gdy pętla się zatrzyma z powodu NULL, sprawdzamy status błędu:
if (errno != 0) {
    // Wystąpił błąd (np. błąd odczytu z dysku)
    perror("Błąd readdir"); 
} else {
    // Brak błędu - po prostu poprawnie dotarto do końca katalogu
    printf("Koniec katalogu.\n");
}
```

### man 0p dirent.h

```C
ino_t  d_ino       -> identyfikator pliku (numer inode, więcej w man 7 inode)
char   d_name[]    -> nazwa pliku
```

### man 3p fstat

```C
#include <sys/stat.h>	// Tam jest więcej o polach w strukturze stat
int stat(const char *restrict path, struct stat *restrict buf);
int lstat(const char *restrict path, struct stat *restrict buf);
int fstat(int fildes, struct stat *buf);
```

Jeśli `stat` lub `lstat` zawiedzie, zwraca -1 (0 w razie sukcesu). W języku C **jakakolwiek liczba różna od zera jest uznawana za prawdę**, więc warunek if(-1) się spełni i program wyrzuci błąd.

Struktura stat zawiera m.in. informacje o rozmiarze pliku, właścicielu, czy dacie ostatniej modyfikacji. Dostępne są też makra sprawdzające typ pliku. Poniżej znajdują się ważniejsze przykłady takich makr:

Makra przyjmujące `buf->st_mode` (pole typu `mode_t`):
`S_ISREG(m)` – czy mamy do czynienia ze zwykłym plikiem,
`S_ISDIR(m)` – czy mamy do czynienia z katalogiem,
`S_ISLNK(m)` – czy mamy do czynienia z linkiem.

Jedyną różnicą w działaniu funkcji `stat` i `lstat` jest obsługa linków. stat zwróci informacje o pliku, do którego dany link prowadzi, natomiast lstat zwróci informacje o samym linku.

```C
struct stat buffer;
fstat(fildes, &buffer);
```

Kiedy piszesz `struct stat buffer;` system automatycznie tworzy dla Ciebie gotowe "pudełko" w pamięci, które pomieści wszystkie informacje o pliku.   
Jednak funkcja `fstat` oczekuje wskaźnika (adresu do pudełka), by móc do niego zajrzeć i je wypełnić. Dlatego dodajemy `&` przed `buffer`. Instrukcja `&buffer` mówi: "weź to moje pudełko i zrób mi wskaźnik na jego adres".

```C
struct stat *buffer;
fstat(fildes, buffer);	// Tu nie trzeba &, bo buffer to już wskaźnik
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

### man 3p scandir

Rozbijmy ten potrójny wskaźnik (`struct dirent ***`) na części logiczne, krok po kroku:   
`struct dirent` – struktura opisująca pojedynczy plik lub katalog.   
`struct dirent *` (1 gwiazdka) – wskaźnik na ten konkretny plik.   
`struct dirent **` (2 gwiazdki) – tablica wskaźników na pliki (czyli cała lista plików znajdujących się w katalogu).   
`struct dirent ***` (3 gwiazdki) – adres, pod którym znajduje się ta tablica wskaźników.

### man 3p getcwd

```C
#include <unistd.h>
char *getcwd(char *buf, size_t size);
```

Kiedy przekazujesz surową tablicę do funkcji takich jak `getcwd(path, MAX_PATH)` czy `strcpy`, funkcja ta wpisuje tekst znak po znaku, a na samym końcu samodzielnie dopisuje `\0`.   
`buf` jest wcześniej zaalokowaną tablicą znaków, do której zostanie zapisana bezwzględna ścieżka do katalogu roboczego. Tablica ta powinna mieć długość co najmniej `size`.   
Funkcja zwraca buf w przypadku sukcesu. W razie niepowodzenia, zwracany jest `NULL`, a `errno` ustawiane jest na odpowiednią wartość.

### man 3p chdir

```C
#include <unistd.h>
int chdir(const char *path);
```

`path` jest ścieżką do nowego katalogu roboczego (może być względna lub bezwzględna), tak jak wiele funkcji systemowych zwracających `int`, funkcja `chdir` zwraca 0 w przypadku sukcesu i inną wartość w razie niepowodzenia.

### man 3p fopen - TRZEBA O TYM PAMIĘTAĆ!!!

```C
FILE *fopen(const char *restrict pathname, const char *restrict mode);
```

1. `r` (read): Otwiera plik wyłącznie do odczytu. Jeśli plik nie istnieje lub nie masz uprawnień do jego odczytania, operacja zakończy się błędem.
2. `w` (write): Otwiera plik do zapisu. Jeśli plik już istnieje, zostaje bezpowrotnie wyczyszczony (jego długość jest "ucinana" do zera). Jeśli plik nie istnieje, system automatycznie go utworzy.
3. `a` (append): Otwiera plik do dopisywania. Jeśli plik nie istnieje, zostanie utworzony. Różnica polega na tym, że niezależnie od tego, co będziesz robić, wszystkie nowe dane będą na siłę dopisywane na samym końcu pliku.

Modyfikator `+` (update - odczyt i zapis jednocześnie):   
Znak `+` dodany do trybu oznacza, że plik jest otwierany do "aktualizacji", czyli pozwala zarówno na czytanie, jak i na pisanie.   
Możesz użyć np. `r+` (odczyt i zapis istniejącego pliku), `w+` (wyczyszczenie/utworzenie pliku z opcją odczytu i zapisu) lub `a+` (dopisywanie na końcu z opcją odczytu).

### man 3p fseek

```C
#include <stdio.h>
int fseek(FILE *stream, long offset, int whence);
int fseeko(FILE *stream, off_t offset, int whence);
```

Funkcja `fseek` przyjmuje wskaźnik `FILE` i pozwala nam przesunąć się na odpowiednie miejsce w pliku. Wyjątkiem jest plik otwarty w trybie “a” - append, który **niezmiennie wskazuje na koniec treści niezależnie od wywołań** `fseek`. Poza tym przypadkiem, tuż po otwarciu, kursor pliku wskazuje na pierwszy bajt.

`stream` jest wyżej wymienionym identyfikatorem strumienia pliku   
`offset` określa liczbę bajtów o którą chcemy się przesunąć   
`whence` mówi o tym jaki punkt odniesienia powinniśmy przyjąć w momencie przesunięcia. Może przyjmować następujące wartości:
1. `SEEK_SET` - punktem odniesienia jest początek pliku, funkcja ustawia kursor pliku na offset-ym bajcie pliku.
2. `SEEK_CUR` - przesunięcie relatywne do obecnego kursora pliku, funkcja ustawia kursor o offset bajtów do przodu (do tyłu w przypadku wartości ujemnej).
3. `SEEK_END` - punktem odniesienia jest koniec pliku. Kursor pliku będzie wskazywał na dane tylko jeśli wartość `offset` jest ujemna.
Dla wartości `offset` **równej 0 kursor pliku ustawiony jest na bajt po ostatnim bajcie pliku**. Odczytanie pozycji kursora funkcją `ftell` podaje wtedy dokładny rozmiar pliku w bajtach. Operacja ta pozwala programiście **zaalokować dokładną ilość bajtów która będzie potrzebna na wczytanie** całego pliku.

**W programie nie wypełniamy pliku zerami, dzieje się to automatycznie ponieważ gdy zapisujemy coś poza aktualnym końcem pliku system automatycznie dopełnia lukę** zerami. Co więcej, jeśli tych zer ciągiem jest sporo to nie zajmują one sektorów dysku!

### man 3p fread

```C
size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
```

`ptr` - bufor w który będą zapisywane dane   
`size` - rozmiar nieprzerwanych elementów do wczytania   
`nitems` - liczba elementów do wczytania   
`stream` - wskaźnik pozyskany z fopen   
Zwrócona wartość oznacza liczbę elementów wczytanych z sukcesem. Będzie ona **mniejsza niż `nitems` w przypadku błędu lub zakończenia pliku**. Można pomyśleć, że podział wczytanych danych na elementy jest niepotrzebną komplikacją, ale daje to pewną korzyść w przypadku gdy wczytywany plik składa się z pewnych rekordów których nie chcemy przerywać w połowie (np. 4-bajtowe zmienne całkowitoliczbowe `int`). W przypadku niepełnego odczytu nie musimy obliczać ile obiektów wczytaliśmy poprawnie ani cofać kursora pliku aby wczytać niepełny rekord jeszcze raz. W momencie gdy pierwsze wywołanie zwróciło `n` wystarczy wywołać funkcję jeszcze raz z przesuniętym buforem `ptr+size*n` oraz liczbą elementów `nitems-n`.

### man 3p fclose

```C
int fclose(FILE *stream);
```

### man 3p unlink

```C
#include <unistd.h>
int unlink(const char *path);

#include <fcntl.h>
int unlinkat(int fd, const char *path, int flag);
```

Jak działa `unlink`:   
Zmniejszenie licznika dowiązań: Głównym zadaniem funkcji `unlink` jest usunięcie wpisu (dowiązania) o danej ścieżce z jej katalogu macierzystego oraz zmniejszenie tak zwanego licznika dowiązań (link count) dla tego pliku.   
Aby system operacyjny faktycznie zwolnił (wyczyścił) przestrzeń na dysku zajmowaną przez plik, muszą zostać spełnione jednocześnie dwa warunki:
1. Licznik dowiązań musi spaść do zera (czyli żadna inna ścieżka w systemie nie wskazuje już na ten plik).
2. Żaden działający proces w komputerze nie ma w tej chwili otwartego tego pliku (np. funkcja `fopen` nie trzyma go w trybie odczytu).

Usuwanie plików w użyciu (Opóźnione usunięcie):   
To jest najważniejsza cecha systemów UNIX. Jeśli wywołasz unlink na pliku, który jest w tej samej chwili otwarty i używany przez jakiś proces, funkcja `unlink` zakończy się sukcesem od razu i usunie ścieżkę z katalogu (plik zniknie z widoku komendy ls). Jednak fizyczne usunięcie zawartości tego pliku z dysku zostanie odroczone aż do momentu, gdy ostatni proces wywoła funkcję `close()` na tym pliku i go zamknie.

Symlinki (Dowiązania symboliczne):   
Jeśli ścieżka podana do `unlink` jest tylko symbolicznym skrótem (symlinkiem), usunięty zostaje wyłącznie ten skrót. Zasadniczy plik lub katalog docelowy pozostaje całkowicie nienaruszony.

Usuwanie katalogów:   
Parametr przekazany do funkcji nie może być ścieżką do katalogu (folderu). Usunięcie katalogu za pomocą unlink jest zarezerwowane wyłącznie dla procesów z uprawnieniami superużytkownika (roota). Aplikacje powinny używać do tego celu funkcji `rmdir()`.

Jak działa unlinkat:   
Druga funkcja w dokumentacji to rozszerzona, bezpieczniejsza wersja unlink z końcówką at (jak at directory).   
Bezpieczna nawigacja (Omijanie Race Conditions):   
`unlinkat` stworzono, by zapobiegać błędom synchronizacji (race conditions). Jeżeli podasz jej ścieżkę względną, usunięcie pliku odbędzie się w oparciu o wcześniej otwarty deskryptor pliku (deskryptor folderu), do którego przypisana jest zmienna fd, a nie względem bieżącego, globalnego katalogu roboczego programu. Gwarantuje to, że plik zostanie usunięty z prawidłowego miejsca, nawet jeśli jakiś inny program w tle zmieni układ folderów.

Usuwanie katalogów (Flaga `AT_REMOVEDIR`):   
W przeciwieństwie do zwykłego `unlink`, `unlinkat` potrafi legalnie i bezpiecznie usunąć również katalog, zachowując się jak funkcja `rmdir()`. Wystarczy jako trzeci parametr (flag) przekazać jej specjalną flagę systemową `AT_REMOVEDIR`.   
Zarówno `unlink`, jak i `unlinkat` zwracają wartość 0 w przypadku poprawnego usunięcia dowiązania lub -1 w razie błędu, odpowiednio ustawiając zmienną errno (np. na EACCES dla braku uprawnień). 

### man 3p umask

```C
#include <sys/stat.h>
mode_t umask(mode_t cmask);
```

Zasada działania: `umask` to systemowa "czarna lista". Określa, jakie uprawnienia należy zabrać (zablokować) przy tworzeniu pliku, a nie jakie mu dać.   
Wzór `umask(~perms & 0777)`:   
`perms` – uprawnienia, które fizycznie chcesz uzyskać na dysku (np. `0777`).   
`~` (negacja) – tłumaczy Twoje chęci na język blokad. Jeśli chcesz dać uprawnienia na wszystko, odwraca bity tak, aby kazać systemowi "zablokować nic" (`0000`).   
`& 0777` (sekator) – ucina niewidoczne zera i jedynki z 32-bitowej zmiennej pamięci, gwarantując, że do systemu trafi czyste i zrozumiałe 9 bitów uprawnień.

Zapisywanie i przywracanie:   
Funkcja `umask()` zawsze zwraca starą listę blokad w momencie jej zmiany.   
`mode_t stara = umask(0000);` wyłącza filtry i chowa starą "tarczę" (np. `0022`) do zmiennej.
`umask(stara);` nie wykonuje żadnego odejmowania. Po prostu nadpisuje obecne ustawienia, przywracając starą "czarną listę" zakazów, żeby inne pliki tworzone w programie miały domyślną ochronę.

### man 3p getopt

```C
#include <unistd.h>
int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int opterr, optind, optopt;
```

`optstring`: To tekst (np. `":abf:o:"`), w którym definiujesz, jakich flag oczekujesz. Jeśli po literze znajduje się dwukropek (`:`), oznacza to, że opcja wymaga podania wartości (argumentu). Jeśli dwukropek jest na samym początku całego ciągu, wycisza to domyślne systemowe komunikaty o błędach, pozwalając programiście obsłużyć je samodzielnie.   
`optarg`: Zewnętrzna zmienna tekstowa (wskaźnik). Jeśli opcja wymaga wartości (np. `-f nazwa.txt`), po jej znalezieniu getopt automatycznie ustawi wskaźnik `optarg` tak, by celował prosto w ten argument.   
`optind`: Licznik oznaczający indeks następnego elementu w tablicy `argv`, który czeka na przetworzenie. System inicjalizuje go wartością 1 i samodzielnie zwiększa w miarę działania funkcji.   
`optopt`: Jeśli użytkownik poda nierozpoznaną flagę lub zapomni wpisać dla niej wartości, getopt zapisuje ten błędny znak właśnie w zmiennej `optopt`, aby można go było wypisać w komunikacie błędu.   
Zwracana wartość: Gdy funkcja **pomyślnie przeanalizuje wszystkie dostępne opcje zaczynające się od myślnika**, zwraca ostatecznie wartość `-1`.

### man 3p mkdir

```C
#include <sys/stat.h>
int mkdir(const char *path, mode_t mode);
#include <fcntl.h>
int mkdirat(int fd, const char *path, mode_t mode);
```

Należy pamiętać, że w przypadku katalogów niezbędne są uprawnienia do wykonania (`x`), które w tym wypadku oznaczają możliwość przejście przez katalog i dostęp do plików i katalogów wewnątrz niego.

### man 3p chmod

```C
#include <sys/stat.h>
int chmod(const char *path, mode_t mode);
#include <fcntl.h>
int fchmodat(int fd, const char *path, mode_t mode, int flag);
```

Przy użyciu wspomnianych wcześniej funkcji `stat` i `lstat` możemy sprawdzać uprawnienia pliku.

R (Read - odczyt), W (Write - zapis), X (Execute - wykonanie).   
USR (User/Właściciel), GRP (Grupa), OTH (Others/Inni).   
Przykład: `S_IRUSR | S_IWUSR` to odczyt i zapis dla właściciela (odpowiednik liczbowy 0600).   
Skróty: Np. `S_IRWXU` nadaje od razu pełen pakiet (odczyt, zapis, wykonanie) dla właściciela (odpowiednik 0700).

### man 3p access

```C
#include <unistd.h>
int access(const char *path, int amode);
#include <fcntl.h>
int faccessat(int fd, const char *path, int amode, int flag);
```

Należy pamiętać, że żeby zmienić uprawnienia nasz proces musi mieć uprawnienia do zapisu danego pliku. Wygodnym sposobem, żeby sprawdzić, czy plik istnieje i mamy odpowiednie uprawnienia jest funkcja `access`.

Należy uważać, bo parametr amode nie jest tym samym co `mode` w `chmod` i `mkdir`. Przyjmuje jedną z czterech flag lub ich kombinację (przez `|`) w zależności od tego, co chcemy sprawdzić: `F_OK` (istnienie), `R_OK` (odczyt), `W_OK` (zapis), `X_OK` (wykonanie).

### man 3p link

```C
#include <unistd.h>
int link(const char *path1, const char *path2);
#include <fcntl.h>
int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag);
```

Służy do tworzenia hardlinków. Ma ich nie być na labach.

### man 3p readlink

```C
#include <unistd.h>
ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);
#include <fcntl.h>
ssize_t readlinkat(int fd, const char *restrict path, char *restrict buf, size_t bufsize);
```

Używamy `readlink` do sprawdzenia, na jaki plik wskazuje dany symlink.
Istotne w użyciu tej funkcji jest fakt, że sami musimy zaalokować bufor `buf`, w którym umieści wynik.

```C
char buf[1024]; // Na stercie size_t buf_size = 1024; char *buf = malloc(buf_size); free(buf);
ssize_t len;
...
if ((len = readlink("/modules/pass1", buf, sizeof(buf)-1)) != -1)
	buf[len] = '\0';
```

Dowiązania symboliczne są tak naprawdę specjalnym rodzajem pliku, zawierającym ścieżkę do innego pliku. Jeżeli plik na który wskazuje dowiązanie symboliczne zmieni nazwę takie dowiązanie przestanie być poprawne - plik na który wskazuje przestaje istnieć.

Mamy dwie funkcje `stat` i `lstat` - `lstat` zwróci informację o symlinku jako o pliku, natomiast `stat` o pliku na który wskazuje.

### man 3p symlink

```C
#include <unistd.h>
int symlink(const char *path1, const char *path2);
#include <fcntl.h>
int symlinkat(const char *path1, int fd, const char *path2);
```

Jej wywołanie tworzy dowiązanie symboliczne o ścieżce path2 do pliku o nazwie path1 (czyli `path2->path1`, `->` oznacz ,,wskazuje na’’). W terminalu mamy bezpośredni odpowiednik tej funkcji `ln -s <TARGET> <PATH>` (**man 1 ln**).   

Rozważ następujący przypadek:   
W naszym katalogu roboczym mamy dwa podkatalogi `a` oraz `b`. W katalogu `a` tworzymy plik `file.txt`. Chcemy teraz stworzyć dowiązanie symboliczne do tego pliku w katalogu `b` o nazwie np. `file.txt_bak` Wywołujemy więc `ln -s a/file.txt b/file.txt_bak`. Czy to zadziała poprawnie?   
Nie, nie zadziała! Zostanie stworzone dowiązanie `b/file.txt_bak` jednak będzie ono wskazywać na względną ścieżkę `a/file.txt`, która z punktu widzenia pliku `b/file.txt_bak` jest niepoprawna, bo dla niego ścieżka do tego pliku to `../a/file.txt`.
Żeby poprawnie stworzyć dowiązanie z naszego katalogu roboczego musimy wywołać `ln -s ../a/file.txt b/file.txt_bak` - wygląda nieintuicyjnie na pierwszy rzut oka, ale po prostu trzeba pamiętać, że ścieżka docelowa jest zawsze z punktu widzenia dowiązania. Alternatywnie możemy wejść do katalogu b i stamtąd wywołać `ln -s ../a/file.txt file.txt_bak`. Aby wygodnie podejrzeć na jaki plik wskazuje dowiązania w danym katalogu najlepiej użyć `ls -l`.

Zawsze możesz stworzyć dowiązanie oparte na ścieżce bezwzględnej (od samego początku dysku, np. `ln -s /home/student/a/file.txt b/file.txt_bak`, ale na labach raczej nie, bo trzeba to przesyłać, więc chodzi o relację miedzy folderami.

### buforowanie

1. Zwykły printf (Standardowe wyjście - stdout)   
Ma bufor (poczekalnię). Nie wypisuje od razu!   
W terminalu: Wyrzuca tekst na ekran dopiero, gdy zobaczy znak nowej linii (`\n`). Bez `\n` ekran jest pusty, aż program dobiegnie końca.   
Zapis do pliku: Czeka, aż zbierze "dużą paczkę" danych. Jeśli przerwiesz program (`Ctrl+C`) w trakcie działania, pusty bufor = utrata danych!

2. Debugowanie i błędy (fprintf(stderr, ...))   
Brak bufora. Wyrzuca tekst natychmiast!   
Do czego: Używaj do debugowania (np. sprawdzania czy program wszedł w pętlę) oraz wypisywania błędów. Jeśli program nagle zginie, masz pewność, że to wywołanie zdążyło wyświetlić się na ekranie.   
Składnia: `fprintf(stderr, "Moja zmienna to: %d\n", i);`

3. Przekierowania w terminalu (Strzałki)   
`./program > plik` – wyrzuca normalne wyniki (printf) do pliku. Na ekranie nic nie widać, terminal milczy.   
`./program 2> plik` – wyrzuca błędy i logi (stderr) do pliku. Ważne: 2> piszemy razem, bez spacji!

# Zadanie z lab1

### C

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

### Makefile

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

### Polecenie

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
