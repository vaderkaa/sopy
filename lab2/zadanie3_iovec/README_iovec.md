Oto Twój "żelazny schemat" na każde zadanie z `readv`/`writev` i plikami binarnymi, który możesz sobie zapisać. Opiera się dokładnie na tym, co właśnie zrobiliśmy:

### Żelazny Schemat na Pliki Binarne w C

**1. Analiza struktur i ich wielkości**

* **Nagłówek:** Najczęściej zawiera `char magic[4]` (lub podobny identyfikator) i jakąś zmienną liczbową (np. `count`). Musi mieć sztywny rozmiar (np. 8 bajtów). *Nigdy* nie stawiaj w nim znaku `\0` dla zmiennej `magic`.
* **Rekord/Pozycja:** Jeśli to zbiór (np. towary, liczby), określ jego rozmiar (np. 36 bajtów lub 4 bajty).
* **Wzór pliku:** Całkowity rozmiar poprawnego pliku na dysku to zawsze: `Wielkość nagłówka + (N * Wielkość jednego rekordu)`.

**2. Scenariusz ZAPISU (Gather - `writev`)**

* **Otwarcie:** `open(..., O_WRONLY | O_CREAT | O_TRUNC, 0644)` – zawsze z opcją `O_TRUNC`, żeby usunąć stare dane z pliku.
* **Przygotowanie Nagłówka:** Tworzysz zmienną nagłówka w pamięci, wpisujesz "magiczne" słowo przez `memcpy` i odpowiednią wartość `count`.
* **Przygotowanie Danych:** Tworzysz tablice/zmienne dla rekordów. Jeśli tekst wymaga "dopełnienia zerami", używasz `memset(..., 0, ...)` albo `strncpy(..., limit)` i zamykasz ostatni bajt zerem.
* **Zbudowanie wektorów:** Deklarujesz tablicę `struct iovec iov[X]`. Wypełniasz indeksy (`iov_base` - adres, `iov_len` - ile bajtów). Z reguły `iov[0]` to nagłówek, a kolejne to rekordy.
* **Akcja:** Wywołujesz `writev(fd, iov, liczba_wektorow)`.

**3. Scenariusz ODCZYTU (Scatter - `readv`)**

* **Otwarcie i fstat:** `open(..., O_RDONLY)`, a następnie `fstat(fd, &st)`. To absolutny fundament przed zrobieniem czegokolwiek z istniejącym plikiem.
* **Walidacja wagi i wyliczenie N:** Sprawdzasz, czy `st.st_size` pozwala na nagłówek i czy zgadza się wzór modulo (np. `(st.st_size - 8) % rozmiar_rekordu == 0`). Wyliczasz `N`.
* **Alokacja:** Alokujesz miejsce w pamięci (`malloc`) na te `N` wyliczonych rekordów.
* **Zbudowanie wektorów:** Tworzysz tablicę `iovec` dokładnie tak samo jak przy zapisie. Wskazujesz na przygotowane "puste" miejsca w pamięci.
* **Akcja i walidacja:** Strzelasz `readv(fd, iov, liczba_wektorow)`. Dopiero gdy dane wylądują w pamięci, walidujesz wartość `magic` (np. przez `strncmp`) i sprawdzasz, czy wczytane z nagłówka `count` odpowiada wyliczonemu matematycznie `N`.

Trzymając się tej kolejności (zwłaszcza matematyki z `fstat` przy odczycie i ręcznego dbania o dopełnianie zerami w strukturach przy zapisie), nie "wyłożysz się" na żadnym tego typu kolokwium!
