//==============================================================
// Projekt : UP2Stream Display
// Plik    : PolishTime.h
//
// Opis:                                                        // Description:
//
// Funkcje związane z obliczaniem aktualnego czasu obowiązującego // Functions for calculating the current time applicable
// w Polsce na podstawie czasu odebranego z modułu Up2Stream.   // in Poland based on the time received from the Up2Stream
//                                                              // module.
//
//==============================================================

#ifndef POLISHTIME_H
#define POLISHTIME_H


#include "Up2StreamClient.h"


//==============================================================
// Struktura czasu polskiego.                                   // Polish time structure.
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
// Sprawdzenie, czy dla podanej daty i godziny UTC obowiązuje    // Checks whether daylight saving time is in effect in
// w Polsce czas letni.                                         // Poland for the specified UTC date and time.
//==============================================================

bool isPolishSummerTime(
    int year,
    int month,
    int day,
    int hour);


//==============================================================
// Konwersja czasu odebranego z Up2Stream na czas polski.       // Converts the time received from Up2Stream to Polish time.
//==============================================================

PolishTime convertToPolishTime(
    const Up2StreamTime& source);


#endif