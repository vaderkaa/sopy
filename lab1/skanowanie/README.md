To jest klasyczny kod do rekurencyjnego chodzenia po katalogach (to dokładnie to Zadanie 4 z dowiązaniami, o którym rozmawialiśmy na samym początku).
Oto krótka notatka, co tu się dzieje:
1. To, co kod robi obecnie (aktywna część):
 * Zaczyna w main od wejścia do folderu "biuro" za pomocą funkcji scan_dir.
 * Czyta wszystko, co jest w tym folderze (pomijając . i .., żeby nie zapętlić się w nieskończoność).
 * Rozpoznaje, z czym ma do czynienia za pomocą lstat.
 * Kluczowy moment: Jeśli znajdzie kolejny folder (S_ISDIR), funkcja wywołuje samą siebie dla tego nowego folderu. Dzięki temu program "nurkuje" w głąb każdego podkatalogu, dopóki nie odwiedzi absolutnie wszystkich plików w "biurze".
2. To, co robi zablokowana (zakomentowana /* ... */) część:
 * W main: Tworzy główny folder archiwum z podziałem na ksiegowe i zarzadowe.
 * W scan_dir: Jeśli program natrafi na zwykły plik (S_ISREG), używa znanego Ci już strstr do sprawdzenia, czy w ścieżce pliku pada słowo "ksiegowosc" lub "zarzad".
 * Jeśli tak, tworzy w nowym archiwum dowiązanie symboliczne (symlink - taki systemowy "skrót"), które wskazuje na ten oryginalny plik.
Podsumowując: To gotowy skrypt, który przeszukuje całą firmową strukturę folderu "biuro" i automatycznie tworzy wygodne skróty do ważnych dokumentów księgowych i zarządowych w jednym zbiorczym archiwum.

Ten kod to uniwersalny "gotowiec" (boilerplate) do każdego zadania, które polega na przeszukiwaniu folderów i podfolderów (tzw. przechodzenie rekurencyjne). Na kolokwium jego szkielet pozostaje zawsze taki sam, a Ty zmieniasz tylko środek – to, co program ma zrobić po znalezieniu konkretnego pliku.
Oto jak dokładnie działa i jak go modyfikować w zależności od polecenia:
1. Żelazny szkielet (tego nie zmieniasz)
 * opendir i readdir: Otwierają folder i w pętli while po kolei "biorą do ręki" każdy element, który się w nim znajduje.
 * Ochrona przed nieskończonością (strcmp): Elementy . (obecny folder) i .. (folder wyżej) są w każdym katalogu. Musisz je pominąć (continue;), inaczej program wszedłby do folderu ., w nim znów do ., i tak aż do zawieszenia komputera.
 * Sklejanie ścieżki: Program musi wiedzieć, gdzie dokładnie leży plik. Kod tworzy tablicę char file_path[1024] i skleja ze sobą: ścieżka_katalogu + / + nazwa_pliku.
 * lstat: Pobiera "metryczkę" pliku (wielkość, uprawnienia, typ) i zapisuje ją do struktury st. Od tego momentu wiesz o pliku wszystko.
2. Gdzie modyfikujesz kod pod konkretne zadanie?
Kluczowy jest moment, w którym sprawdzasz typ pliku za pomocą makr S_IS...:
 * Sekcja S_ISREG(st.st_mode) (Zwykłe pliki):
   Tu wstawiasz główną logikę z treści zadania.
   * Szukanie po nazwie: Obecnie zakomentowany kod używa strstr(file_path, "ksiegowosc"), by szukać słowa w ścieżce. Jeśli prowadzący każe znaleźć pliki z rozszerzeniem .txt, zmienisz to na strstr(dp->d_name, ".txt").
   * Szukanie po rozmiarze: Jeśli masz znaleźć pliki mniejsze niż 50 bajtów, wpiszesz: if (st.st_size < 50).
   * Akcja: Obecnie w kodzie jest symlink(target, link), czyli tworzenie skrótu. Jeśli polecenie każe usunąć plik, wpiszesz tam po prostu unlink(file_path);. Jeśli każe wypisać rozmiar, dasz printf("%ld", st.st_size);.
 * Sekcja S_ISDIR(st.st_mode) (Katalogi):
   Tu znajduje się wywołanie rekurencyjne: scan_dir(file_path);. Gdy program widzi folder, wstrzymuje na chwilę obecną pracę i odpala sam siebie dla tego nowego folderu. Kiedy go w całości przeszuka, wraca do poprzedniego. Tego na kolokwium nie ruszasz – to gwarantuje przeszukanie całego drzewa.
3. Pułapka z dowiązaniami (symlink)
Jeśli zadanie faktycznie będzie wymagało tworzenia dowiązań symolicznych, zwróć uwagę na te linijki:
char target[1024] = "../../";
strcat(target, file_path);

Dowiązanie symboliczne to tylko głupi plik tekstowy ze ścieżką w środku. Ponieważ dowiązanie jest tworzone głęboko w nowym folderze (np. archiwum/ksiegowe/), musi mieć wklejone ../../ (wyjdź dwa foldery wyżej), żeby stamtąd "zobaczyć" oryginalny plik leżący gdzieś w głównym folderze biura.
W skrócie na laby: Kopiujesz ten kod, odnajdujesz linijkę z S_ISREG i to w jej wnętrzu piszesz warunki z polecenia (np. filtrując rozmiary przez st.st_size lub nazwy przez strstr).

Podczas kolokwium w sekcji odpowiedzialnej za tworzenie dowiązań najprawdopodobniej będziesz musiała zmodyfikować jeden z tych czterech elementów, w zależności od polecenia:
 * 1. Głębokość ścieżki powrotnej (target)
   Obecnie w kodzie masz char target[1024] = "../../";. To ../../ cofa Cię o dwa foldery w górę, ponieważ Twój skrót powstaje w folderze archiwum/ksiegowe/ (czyli dwa poziomy poniżej miejsca odpalenia programu).
   Zmiana na labach: Jeśli polecenie każe wrzucić skróty do pojedynczego folderu np. kopie/ (jeden poziom głębokości), musisz to zmienić na ../. Jeśli folder będzie potrójny (a/b/c/), wpiszesz ../../../.
 * 2. Ścieżka bezwzględna zamiast z kropkami
   Prowadzący może zaznaczyć: "Dowiązanie ma wskazywać na bezwzględną ścieżkę do pliku" (czyli taką zaczynającą się od /home/user/...).
   Zmiana na labach: Zamiast bawić się w ręczne doklejanie ../../, użyjesz funkcji realpath(file_path, target), która sama przetłumaczy aktualną ścieżkę pliku na pełną i absolutną.
 * 3. Nazwa tworzonego dowiązania (link)
   W obecnym kodzie skrót nazywa się dokładnie tak samo, jak oryginalny plik, bo doklejasz samo dp->d_name.
   Zmiana na labach: Prowadzący może poprosić o dodanie przedrostka, np. skrót ma się nazywać link_nazwapliku.txt. Musisz wtedy odpowiednio użyć strcat, żeby skleić to w poprawnej kolejności przed wywołaniem symlink.
 * 4. Zmiana na twarde dowiązanie (Hard link)
   Polecenie może wyraźnie wymagać stworzenia "twardego dowiązania", a nie "symbolicznego".
   Zmiana na labach: Zamieniasz wywołanie symlink(target, link) na link(target, link). Co ważne, przy twardym dowiązaniu w ogóle nie używa się kopek powrotnych (../../), tylko podaje wprost ścieżkę do oryginalnego pliku (file_path).
