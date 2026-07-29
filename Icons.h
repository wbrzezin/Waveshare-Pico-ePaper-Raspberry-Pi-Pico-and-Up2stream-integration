//==============================================================
// Projekt : UP2Stream Display
// Plik    : Icons.h
//
// Deklaracje ikon interfejsu użytkownika.
//==============================================================

#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "IconBitmaps.h"

//--------------------------------------------------------------
// Identyfikatory dostępnych ikon.
//--------------------------------------------------------------
enum class IconId
{
    None,

    Spotify,
    Bluetooth,
    AirPlay,
    DLNA,
    NAS,
    USB,
    Radio,

    Play,
    Pause,
    Stop,

    Next,
    Previous,

    Volume,
    Mute,

    Ethernet,

    WiFi0,
    WiFi1,
    WiFi2,
    WiFi3
};

//--------------------------------------------------------------
// Rysowanie ikony.
//
// Parametry:
//
// display
//      Obiekt wyświetlacza zgodny z biblioteką Adafruit_GFX.
//
// id
//      Identyfikator ikony.
//
// x, y
//      Współrzędne lewego górnego rogu.
//
// Zwraca:
//
// true
//      Ikona została narysowana.
//
// false
//      Nieobsługiwany identyfikator.
//--------------------------------------------------------------

bool drawIcon(
    Adafruit_GFX& display,
    IconId id,
    int x,
    int y);

#endif