#include <stdio.h>
#include <string.h>

int main() 
{
    // 1. Cięcie na dokładnie dwie części za pomocą strstr (bez pętli)
    char paczka[] = "numpy==1.0.0";
    char *eq = strstr(paczka, "==");
    if (eq != NULL) {
        *eq = '\0'; 
        printf("Czesc 1: %s, Czesc 2: %s\n", paczka, eq + 2);
    }

    // 2. Dzielenie na nieznaną ilość kawałków po spacji za pomocą strtok
    char lista[] = "numpy pandas matplotlib keras";
    char *wyraz = strtok(lista, " ");
    while (wyraz != NULL) {
        printf("Wyraz: %s\n", wyraz);
        wyraz = strtok(NULL, " "); 
    }

    // 3. Dzielenie przy powtarzającym się separatorze (strtok ignoruje nadmiarowe znaki =)
    char dziwny_tekst[] = "numpy==1.0.0==dodatek==linux";
    char *fragment = strtok(dziwny_tekst, "=");
    while (fragment != NULL) {
        printf("Kawalek: %s\n", fragment);
        fragment = strtok(NULL, "="); 
    }

    return 0;
}
