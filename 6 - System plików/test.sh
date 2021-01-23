#!/bin/sh

VFS_FILE_NAME='drive.vfs'

echo 'Zaczynamy od utworzenia dysku o rozmiarze 10240 bajtow. Taki dysk powinien posiadac 4 inody + 4 bloki po 2kb + superblock'
./vfs $VFS_FILE_NAME create 10240
echo 'Drukujemy na ekran zrzut danych'
./vfs $VFS_FILE_NAME dump

echo 'Zapisujemy do srodka plik o rozmiarze 2kb. Powinien zajac dokladnie jeden, pierwszy blok'
./vfs $VFS_FILE_NAME push 2kb.txt 2kb
echo 'Wyswietlamy zrzut, aby potwierdzic nasze obawy'
./vfs $VFS_FILE_NAME dump
echo 'Dodajemy plik o rozmiarze 2kb + 1. Powinien zajac dwa bloki, drugi i trzeci, czyli o numerach 1 i 2'
./vfs $VFS_FILE_NAME push 2k1b.txt 2k1b
echo 'Wyswietlamy stan'
./vfs $VFS_FILE_NAME dump

echo 'Wyswietlamy liste plikow'
./vfs $VFS_FILE_NAME list

echo 'Teraz pokazuje, ze proba dodania pliku o rozmiarze 2kb + 1 skonczy sie bledem - za malo miejsca'
./vfs $VFS_FILE_NAME push 2k1b.txt 2k1b_2

echo 'Proba dodania pliku o rozmiarze jednego bloku, ale o nazwie 2kb - tez skonczy sie bledem - plik o tej nazwie juz istnieje, a domyslnie nie pozwalamy kopiowac'
./vfs $VFS_FILE_NAME push 2kb.txt 2kb

echo 'Pokazemy teraz przyklad fragmentacji. Usuniemy plik 2kb i dodamy drugi plik o rozmiarze 2kb + 1. Powinien otoczyc pierwszy plik.'
./vfs $VFS_FILE_NAME remove 2kb
./vfs $VFS_FILE_NAME push 2k1b.txt 2k1b_2

echo 'Potwierdzamy to zrzutem'
./vfs $VFS_FILE_NAME dump

echo 'Ostatecznie usuniemy srodkowy plik, dodamy jeden pusty, a nastepnie wyciagniemy wszystkie pliki z systemu wirtualnego do naszego'
./vfs $VFS_FILE_NAME remove 2k1b
./vfs $VFS_FILE_NAME push 0b.txt 0b

./vfs $VFS_FILE_NAME pull 2k1b_2 2k1b_2.downloaded
./vfs $VFS_FILE_NAME pull 0b 0b.downloaded

echo 'Ostatni zrzut powierzchni i konczymy prace'
./vfs $VFS_FILE_NAME dump