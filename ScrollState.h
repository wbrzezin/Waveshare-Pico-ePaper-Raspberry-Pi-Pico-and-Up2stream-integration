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

struct ScrollState
{
    //----------------------------------------------------------
    // Aktualne przesunięcie tekstu.
    //----------------------------------------------------------

    int offset = 0;

    //----------------------------------------------------------
    // Szerokość tekstu w pikselach.
    //----------------------------------------------------------

    int textWidth = 0;

    //----------------------------------------------------------
    // Szerokość dostępnego obszaru.
    //----------------------------------------------------------

    int areaWidth = 0;

    //----------------------------------------------------------
    // Czy tekst wymaga przewijania.
    //----------------------------------------------------------

    bool enabled = false;

    //----------------------------------------------------------
    // Znacznik czasu ostatniego przesunięcia.
    //----------------------------------------------------------

    uint32_t lastUpdate = 0;

    //----------------------------------------------------------
    // Czy trwa pauza.
    //----------------------------------------------------------

    bool pause = false;

    //----------------------------------------------------------
    // Początek pauzy.
    //----------------------------------------------------------

    uint32_t pauseStart = 0;
};

#endif