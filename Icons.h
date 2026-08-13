//==============================================================
// Projekt : UP2Stream Display
// Plik    : Icons.h
//
// Deklaracje ikon interfejsu użytkownika.                      // Declarations of user interface icons.
//==============================================================

#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "IconBitmaps.h"

//==============================================================
// Identyfikatory ikon wykorzystywanych przez interfejs.        // Identifiers of icons used by the interface.
//
// W aktualnej wersji programu stosowane są wyłącznie          // The current version of the program uses only
// ikony sterowania odtwarzaniem.                               // playback control icons.
//==============================================================

enum class IconId
{
    None,

    Play,
    Pause,
    Stop
};

//--------------------------------------------------------------//-----------------------------------------------//
// Rysowanie ikony.                                             // Drawing an icon.
//                                                              //
// Parametry:                                                   // Parameters:
//                                                              //
// display                                                      // display
//      Obiekt wyświetlacza zgodny z biblioteką Adafruit_GFX.  //      Display object compatible with the
//                                                              //      Adafruit_GFX library.
//                                                              //
// id                                                           // id
//      Identyfikator ikony.                                    //      Icon identifier.
//                                                              //
// x, y                                                         // x, y
//      Współrzędne lewego górnego rogu.                        //      Coordinates of the upper-left corner.
//                                                              //
// Zwraca:                                                       // Returns:
//                                                              //
// true                                                         // true
//      Ikona została narysowana.                               //      The icon was drawn.
//                                                              //
// false                                                        // false
//      Nieobsługiwany identyfikator.                           //      Unsupported identifier.
//--------------------------------------------------------------//-----------------------------------------------//

bool drawIcon(
    Adafruit_GFX& display,
    IconId id,
    int x,
    int y);

#endif