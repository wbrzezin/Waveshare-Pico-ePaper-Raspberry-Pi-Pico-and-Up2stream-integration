//==============================================================
// Projekt : UP2Stream Display
// Plik    : PolishTime.h
//
// Opis:
//
// Funkcje związane z obliczaniem czasu obowiązującego w Polsce.
//
// UP2Stream zwraca czas TME z własnym przesunięciem UTC.
// W naszym przypadku nie możemy bezpośrednio użyć pola offset,
// ponieważ UP2Stream nie uwzględnia poprawnie czasu letniego.
//
// Moduł oblicza więc dodatkową korektę dla Polski.
//
// Zimą:
//     UTC+1
//
// Latem:
//     UTC+2
//
//==============================================================

#ifndef POLISHTIME_H
#define POLISHTIME_H

#include "Up2StreamClient.h"

//==============================================================
// Struktura czasu polskiego.
//==============================================================

struct PolishTime
{
    int year = 0;
    int month = 0;
    int day = 0;

    int hour = 0;
    int minute = 0;
    int second = 0;

    bool valid = false;
};


//==============================================================
// Sprawdzenie, czy podana data przypada na okres czasu letniego
// obowiązującego w Polsce.
//
// Zwraca:
//
// true  - czas letni UTC+2
// false - czas zimowy UTC+1
//==============================================================

bool isPolishSummerTime(
    int year,
    int month,
    int day,
    int hour);


//==============================================================
// Konwersja czasu odebranego z UP2Stream na czas polski.
//
// Zakładamy, że czas TME jest czasem bazowym UP2Stream,
// a jego standardowe przesunięcie dla naszego urządzenia
// wynosi UTC+1.
//
// W okresie letnim dodajemy dodatkową godzinę.
//
//==============================================================

PolishTime convertToPolishTime(
    const Up2StreamTime& source);


#endif