//==============================================================
// Projekt : UP2Stream Display
// Plik    : ScreenManager.cpp
//==============================================================

#include "ScreenManager.h"

//==============================================================
// Ustawienie aktywnego ekranu.
//==============================================================

void ScreenManager::setScreen(Screen screen)
{
    currentScreen = screen;
}

//==============================================================
// Zwrócenie aktualnego ekranu.
//==============================================================

Screen ScreenManager::getScreen() const
{
    return currentScreen;
}