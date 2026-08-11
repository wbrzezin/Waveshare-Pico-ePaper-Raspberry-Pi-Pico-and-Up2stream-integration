//==============================================================
// Projekt : UP2Stream Display
// Plik    : PolishTime.h
//
// Opis:
//
// Funkcje związane z obliczaniem aktualnego czasu obowiązującego
// w Polsce na podstawie czasu odebranego z modułu Up2Stream.
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
// Sprawdzenie, czy dla podanej daty i godziny UTC obowiązuje
// w Polsce czas letni.
//==============================================================

bool isPolishSummerTime(
    int year,
    int month,
    int day,
    int hour);


//==============================================================
// Konwersja czasu odebranego z Up2Stream na czas polski.
//==============================================================

PolishTime convertToPolishTime(
    const Up2StreamTime& source);


#endif