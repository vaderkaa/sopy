ZMIENNE ŚRODOWISKOWE W C (Krótka ściąga)
Zmienne środowiskowe to pary "KLUCZ=WARTOŚĆ" (np. USER=student, HOME=/home/student), które system operacyjny przechowuje w tle. Programy w C mogą je czytać, tworzyć i modyfikować.
Wszystkie potrzebne do tego funkcje znajdują się w bibliotece:
#include <stdlib.h>

1. Odczytywanie zmiennej (getenv)
Najczęstsza operacja na kolokwium. Szuka zmiennej po nazwie i zwraca wskaźnik do jej wartości (jako string). Jeśli zmiennej nie ma, zwraca NULL.
char *home_path = getenv("HOME");

if (home_path != NULL) {
    printf("Ścieżka do katalogu domowego to: %s\n", home_path);
} else {
    printf("Błąd: Zmienna HOME nie istnieje!\n");
}

2. Tworzenie lub modyfikowanie zmiennej (setenv)
Przyjmuje trzy argumenty: nazwę, nową wartość oraz "flagę nadpisywania" (1 - nadpisz jeśli już istnieje, 0 - nie ruszaj, jeśli istnieje).
// Stworzy zmienną PORT=8080. Jeśli PORT już istnieje, zmieni jej wartość na 8080 (bo jest 1 na końcu).
if (setenv("PORT", "8080", 1) == -1) {
    ERR("setenv"); // Obsługa błędu, jeśli braknie pamięci
}

// Próba ustawienia, ale NIE NADPISZE wartości, jeśli HOST już istnieje (bo jest 0).
setenv("HOST", "localhost", 0); 

3. Usuwanie zmiennej (unsetenv)
Po prostu kasuje zmienną ze środowiska programu.
if (unsetenv("PORT") == -1) {
    ERR("unsetenv");
}

4. Wypisywanie absolutnie wszystkich zmiennych (environ)
Czasami prowadzący każe przeszukać lub wypisać całe środowisko. Służy do tego wbudowana, globalna tablica wskaźników environ, która na końcu ma NULL. Należy ją zadeklarować słowem extern.
#include <stdio.h>
#include <stdlib.h>

extern char **environ; // Pobieramy tablicę od systemu operacyjnego

int main() {
    int i = 0;
    // Pętla kręci się, aż natrafi na pusty wskaźnik (NULL)
    while (environ[i] != NULL) {
        printf("Zmienna nr %d: %s\n", i, environ[i]);
        i++;
    }
    return 0;
}

O czym musisz pamiętać na labach:
 * Zmiany w zmiennych środowiskowych (przez setenv i unsetenv) działają tylko wewnątrz Twojego programu i w procesach potomnych, które on ewentualnie uruchomi. Kiedy program się zakończy, systemowy terminal nie zapamięta tych zmian (zmienne w terminalu pozostaną po staremu).
 * Zawsze sprawdzaj, czy getenv nie zwróciło NULL, zanim spróbujesz użyć pobranego stringa (np. do sklejenia ścieżki w strcat), bo inaczej program natychmiast wyrzuci "Segmentation fault".
