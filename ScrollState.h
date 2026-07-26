//==============================================================
// Projekt : UP2Stream Display
// Plik    : ScrollState.h
//
// Opis:
//
// Struktura przechowująca komplet informacji dotyczących
// przewijania pojedynczego napisu.
//
// Dzięki temu wszystkie parametry animacji znajdują się
// w jednym miejscu i nie zaśmiecają klasy Display.
//
//==============================================================

#ifndef SCROLLSTATE_H
#define SCROLLSTATE_H

#include <Arduino.h>

//==============================================================
// Struktura ScrollState
//
// Przechowuje aktualny stan animacji przewijanego tekstu.
//
//==============================================================

#pragma once

#include <Arduino.h>

//==============================================================
// Stan wyświetlania długiego tekstu
//==============================================================
struct ScrollState
{
    //----------------------------------------------------------
    // Czy tekst wymaga podziału na strony.
    //----------------------------------------------------------
    bool enabled = false;

    //----------------------------------------------------------
    // Gotowe strony tekstu.
    //----------------------------------------------------------
    String pages[10];

    //----------------------------------------------------------
    // Liczba przygotowanych stron.
    //----------------------------------------------------------
    uint8_t pageCount = 0;

    //----------------------------------------------------------
    // Aktualnie wyświetlana strona.
    //----------------------------------------------------------
    uint8_t currentPage = 0;

    //----------------------------------------------------------
    // Czas ostatniej zmiany strony.
    //----------------------------------------------------------
    uint32_t lastUpdate = 0;

    //----------------------------------------------------------
    // Szerokość tekstu (informacja diagnostyczna).
    //----------------------------------------------------------
    uint16_t textWidth = 0;

    //----------------------------------------------------------
    // Dostępna szerokość pola (informacja diagnostyczna).
    //----------------------------------------------------------
    uint16_t areaWidth = 0;
};

#endif