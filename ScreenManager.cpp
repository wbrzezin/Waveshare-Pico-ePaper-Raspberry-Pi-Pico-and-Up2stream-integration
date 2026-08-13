//==============================================================
// Projekt : UP2Stream Display
// Plik    : ScreenManager.cpp
//==============================================================


#include "ScreenManager.h"


//==============================================================
// Ustawienie aktywnego ekranu.                               // Set the active screen.
//==============================================================


void ScreenManager::setScreen(Screen screen)
{
    currentScreen = screen;
}


//==============================================================
// Zwrócenie aktualnego ekranu.                               // Return the current screen.
//==============================================================


Screen ScreenManager::getScreen() const
{
    return currentScreen;
}