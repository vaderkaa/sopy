#!/bin/bash

# Zabezpieczenie: usunięcie starego katalogu, jeśli istnieje (czysty start)
rm -rf manifests

# Tworzenie struktury katalogów
mkdir -p manifests

# Tworzenie prawidłowych plików
echo 'K-017;Gdynia;zboze:120,sol:30'                > manifests/kogga.txt
echo 'S-002; Gdansk ; zboze : 40 , miod pitny:12 '  > manifests/szkuta.txt
echo 'G-100;Gdynia;sol:5'                           > manifests/galeon.txt

# Tworzenie dowiązania symbolicznego
# Uwaga: link wskazuje na plik w tym samym katalogu
ln -s kogga.txt manifests/kogga-link.txt

# Tworzenie plików z celowymi błędami (przypadki brzegowe)
echo 'X-1;Gdynia;sol:5'                             > manifests/zly-id.txt
echo 'K-018;Gdynia;sol:5,,zboze:3'                  > manifests/pusty-towar.txt
echo 'K-019;Gdynia;sol:12abc'                       > manifests/zla-ilosc.txt
echo 'K-020;Gd/ynia;sol:1'                          > manifests/ukosnik.txt
echo 'K-021;Gdynia;sol:1;extra'                     > manifests/za-duzo-pol.txt

echo "Gotowe! Struktura katalogów i pliki zostały wygenerowane w folderze 'manifests'."