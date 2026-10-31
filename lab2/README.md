# LAB 2

### man 3p open

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
