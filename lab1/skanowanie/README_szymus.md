# SOP L1 – szybka ściąga z funkcji POSIX/C

Krótka ściąga na laboratorium: sygnatura, najważniejsze parametry, wartość zwracana i 1–2 zdania o zastosowaniu.

---

## `opendir`

```c
DIR *opendir(const char *name);
```

**Parametry:** `name` – ścieżka do katalogu.  
**Zwraca:** wskaźnik `DIR *` przy sukcesie, `NULL` przy błędzie.

Otwiera katalog do odczytu jego zawartości. Otrzymany `DIR *` przekazujemy później do `readdir()` i na końcu zamykamy przez `closedir()`.

---

## `readdir`

```c
struct dirent *readdir(DIR *dirp);
```

**Parametry:** `dirp` – katalog wcześniej otwarty przez `opendir()`.  
**Zwraca:** wskaźnik do `struct dirent`, albo `NULL` po końcu katalogu lub przy błędzie.

Pobiera kolejny wpis z katalogu. `dp->d_name` zawiera nazwę wpisu; zwykle trzeba pominąć `"."` oraz `".."`.

---

## `stat`

```c
int stat(const char *path, struct stat *buf);
```

**Parametry:** `path` – ścieżka do obiektu, `buf` – struktura, do której zapisane zostaną informacje.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Pobiera informacje o obiekcie pod ścieżką. Jeśli `path` jest symlinkiem, `stat()` podąża za nim i opisuje jego cel.

---

## `lstat`

```c
int lstat(const char *path, struct stat *buf);
```

**Parametry:** takie same jak w `stat()`.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Działa jak `stat()`, ale dla symlinka opisuje sam link, a nie jego cel. Używaj go, gdy chcesz wykryć `S_ISLNK(st.st_mode)`.

---

## `closedir`

```c
int closedir(DIR *dirp);
```

**Parametry:** `dirp` – katalog otwarty przez `opendir()`.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Zamyka strumień katalogu i zwalnia związane z nim zasoby. Każde udane `opendir()` powinno mieć odpowiadające `closedir()`.

---

## `getcwd`

```c
char *getcwd(char *buf, size_t size);
```

**Parametry:** `buf` – bufor na ścieżkę, `size` – jego rozmiar.  
**Zwraca:** `buf` przy sukcesie, `NULL` przy błędzie.

Pobiera bezwzględną ścieżkę aktualnego katalogu roboczego procesu. Przydaje się np. przed `chdir()`, jeśli później chcesz wrócić do poprzedniego katalogu.

---

## `chdir`

```c
int chdir(const char *path);
```

**Parametry:** `path` – ścieżka do nowego katalogu roboczego.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Zmienia katalog roboczy procesu. Po zmianie wszystkie ścieżki względne są interpretowane względem nowego katalogu.

---

## `fopen`

```c
FILE *fopen(const char *pathname, const char *mode);
```

**Parametry:** `pathname` – ścieżka do pliku, `mode` – tryb otwarcia, np. `"r"`, `"w"`, `"a"`.  
**Zwraca:** `FILE *` przy sukcesie, `NULL` przy błędzie.

Otwiera plik jako strumień biblioteki `stdio`. `"r"` czyta, `"w"` tworzy/zeruje plik, `"a"` dopisuje na końcu.

---

## `fseek`

```c
int fseek(FILE *stream, long offset, int whence);
```

**Parametry:** `stream` – otwarty plik, `offset` – przesunięcie, `whence` – np. `SEEK_SET`, `SEEK_CUR`, `SEEK_END`.  
**Zwraca:** `0` przy sukcesie, wartość niezerową przy błędzie.

Przesuwa pozycję odczytu/zapisu w pliku. Pozwala np. ustawić kursor na konkretnym bajcie albo na końcu pliku.

---

## `fprintf`

```c
int fprintf(FILE *stream, const char *format, ...);
```

**Parametry:** `stream` – gdzie pisać, `format` – format jak w `printf()`.  
**Zwraca:** liczbę zapisanych znaków albo wartość ujemną przy błędzie.

Zapisuje sformatowany tekst do wskazanego strumienia. Możesz pisać zarówno do pliku z `fopen()`, jak i do `stdout` czy `stderr`.

---

## `fread`

```c
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
```

**Parametry:** `ptr` – bufor docelowy, `size` – rozmiar jednego elementu, `nitems` – liczba elementów, `stream` – plik.  
**Zwraca:** liczbę poprawnie odczytanych elementów.

Czyta dane binarne z pliku. Jeśli wynik jest mniejszy niż `nitems`, mogło wystąpić EOF albo błąd.

---

## `fclose`

```c
int fclose(FILE *stream);
```

**Parametry:** `stream` – plik otwarty przez `fopen()`.  
**Zwraca:** `0` przy sukcesie, `EOF` przy błędzie.

Zamyka strumień pliku i zwalnia związane z nim zasoby. Każde udane `fopen()` powinno mieć odpowiadające `fclose()`.

---

## `unlink`

```c
int unlink(const char *path);
```

**Parametry:** `path` – ścieżka do pliku lub symlinka.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Usuwa wpis z systemu plików. Dla symlinka usuwa sam link, a nie jego cel.

---

## `mkdir`

```c
int mkdir(const char *path, mode_t mode);
```

**Parametry:** `path` – nazwa/ścieżka katalogu, `mode` – uprawnienia, np. `0755`.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Tworzy nowy katalog. Jeśli katalog już istnieje, zwykle dostaniesz `errno == EEXIST`.

---

## `chmod`

```c
int chmod(const char *path, mode_t mode);
```

**Parametry:** `path` – ścieżka do obiektu, `mode` – nowe uprawnienia, np. `0444`, `0644`, `0755`.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Zmienia uprawnienia pliku lub katalogu. Przykład: `0444` oznacza tylko odczyt dla właściciela, grupy i innych.

---

## `access`

```c
int access(const char *path, int amode);
```

**Parametry:** `path` – ścieżka, `amode` – np. `F_OK`, `R_OK`, `W_OK`, `X_OK`.  
**Zwraca:** `0`, jeśli test przechodzi, `-1` w przeciwnym razie.

Sprawdza istnienie obiektu lub dostępność określonych praw. `F_OK` sprawdza tylko istnienie, a `R_OK/W_OK/X_OK` odpowiednio możliwość odczytu, zapisu i wykonania/przejścia.

---

## `readlink`

```c
ssize_t readlink(const char *path, char *buf, size_t bufsiz);
```

**Parametry:** `path` – ścieżka do symlinka, `buf` – bufor na target, `bufsiz` – rozmiar bufora.  
**Zwraca:** liczbę zapisanych bajtów, `-1` przy błędzie.

Odczytuje dokładny tekst zapisany wewnątrz symlinka. **Nie dopisuje `'\0'`**, więc po sukcesie zwykle robisz `buf[len] = '\0'`, jeśli masz na to miejsce.

---

## `symlink`

```c
int symlink(const char *target, const char *linkpath);
```

**Parametry:** `target` – tekst zapisany w symlinku, `linkpath` – miejsce utworzenia linku.  
**Zwraca:** `0` przy sukcesie, `-1` przy błędzie.

Tworzy dowiązanie symboliczne. Jeśli `target` jest względny, jest interpretowany względem katalogu, w którym znajduje się symlink.

---

## `getline`

```c
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
```

**Parametry:** `lineptr` – wskaźnik na bufor, `n` – aktualny rozmiar bufora, `stream` – plik.  
**Zwraca:** liczbę odczytanych bajtów, `-1` przy EOF albo błędzie.

Czyta całą linię i w razie potrzeby sam alokuje lub powiększa bufor. Typowy start to `char *buf = NULL; size_t size = 0;`, a po zakończeniu trzeba zrobić `free(buf)`.

---

# Najważniejsze pary do zapamiętania

```text
opendir  -> closedir
fopen    -> fclose
getline  -> free(buf)
malloc   -> free
```

```text
lstat -> informacje o samym symlinku
stat  -> informacje o celu symlinka
```

```text
symlink(target, linkpath)

target   = co jest zapisane w linku
linkpath = gdzie link zostaje utworzony
```
