//==============================================================
// Projekt : UP2Stream Display
// Plik    : Icons.cpp
//
// Implementacja modułu ikon.
//
// Na obecnym etapie plik zawiera jedynie szkielet.
// Bitmapy zostaną dodane w kolejnych krokach.
//==============================================================

#include <Adafruit_GFX.h>
#include "Icons.h"
#include "IconBitmaps.h"

#include <GxEPD2_GFX.h>

bool drawIcon(
    Adafruit_GFX& display,
    IconId id,
    int x,
    int y)
{
    const uint8_t* bitmap = nullptr;
    uint8_t width = 0;
    uint8_t height = 0;

    switch (id)
    {
        //------------------------------------------------------
        // PLAY / PAUSE / STOP 
        //------------------------------------------------------
        case IconId::Play:
            bitmap = ICON_PLAY_8;
            width = 8;
            height = 8;
            break;

        case IconId::Pause:
            bitmap = ICON_PAUSE_8;
            width = 8;
            height = 8;
            break;

        case IconId::Stop:
            bitmap = ICON_STOP_8;
            width = 8;
            height = 8;
            break;

        default:
            return false;
    }

    display.drawBitmap(
        x,
        y,
        bitmap,
        width,
        height,
        GxEPD_BLACK);

    return true;
}