//==============================================================
// Projekt : UP2Stream Display
// Plik    : ScrollState.h
//
// Opis:                                                        // Description:
//
// Struktura przechowująca komplet informacji dotyczących       // Structure storing all information related to
// przewijania pojedynczego napisu.                             // scrolling a single text string.
//
// Dzięki temu wszystkie parametry animacji znajdują się       // This keeps all animation parameters
// w jednym miejscu i nie zaśmiecają klasy Display.             // in one place without cluttering the Display class.
//
//==============================================================

#ifndef SCROLLSTATE_H
#define SCROLLSTATE_H

#include <Arduino.h>

//==============================================================
// Struktura ScrollState                                       // ScrollState structure
//
// Przechowuje aktualny stan animacji przewijanego tekstu.      // Stores the current state of the scrolling text animation.
//
//==============================================================

#pragma once

#include <Arduino.h>

//==============================================================
// Stan wyświetlania długiego tekstu                            // Display state of long text
//==============================================================
struct ScrollState
{
    //----------------------------------------------------------
    // Czy tekst wymaga podziału na strony.                     // Whether the text needs to be split into pages.
    //----------------------------------------------------------
    bool enabled = false;

    //----------------------------------------------------------
    // Gotowe strony tekstu.                                    // Prepared text pages.
    //----------------------------------------------------------
    String pages[10];

    //----------------------------------------------------------
    // Liczba przygotowanych stron.                             // Number of prepared pages.
    //----------------------------------------------------------
    uint8_t pageCount = 0;

    //----------------------------------------------------------
    // Aktualnie wyświetlana strona.                            // Currently displayed page.
    //----------------------------------------------------------
    uint8_t currentPage = 0;

    //----------------------------------------------------------
    // Czas ostatniej zmiany strony.                            // Time of the last page change.
    //----------------------------------------------------------
    uint32_t lastUpdate = 0;

    //----------------------------------------------------------
    // Szerokość tekstu (informacja diagnostyczna).              // Text width (diagnostic information).
    //----------------------------------------------------------
    uint16_t textWidth = 0;

    //----------------------------------------------------------
    // Dostępna szerokość pola (informacja diagnostyczna).       // Available field width (diagnostic information).
    //----------------------------------------------------------
    uint16_t areaWidth = 0;
};

#endif