#pragma once

//==============================================================
// Projekt : UP2Stream Display
// Plik    : ScreenManager.h
// Autor   : Waldemar Brzeziński
//
// Opis:
// Zarządzanie aktualnie wyświetlanym ekranem.
//
// W kolejnych etapach klasa będzie odpowiedzialna za
// automatyczne przełączanie pomiędzy ekranem odtwarzacza
// oraz ekranem bezczynności.
//==============================================================

//--------------------------------------------------------------
// Dostępne ekrany aplikacji.
//--------------------------------------------------------------

enum class Screen
{
    //----------------------------------------------------------
    // Ekran odtwarzacza.
    //----------------------------------------------------------

    Player,

    //----------------------------------------------------------
    // Ekran bezczynności.
    //----------------------------------------------------------

    Idle
};

//==============================================================
// Klasa ScreenManager
//==============================================================

class ScreenManager
{
public:

    //----------------------------------------------------------
    // Ustawienie aktywnego ekranu.
    //----------------------------------------------------------

    void setScreen(Screen screen);

    //----------------------------------------------------------
    // Odczyt aktualnego ekranu.
    //----------------------------------------------------------

    Screen getScreen() const;

private:

    //----------------------------------------------------------
    // Aktualnie wyświetlany ekran.
    //----------------------------------------------------------

    Screen currentScreen = Screen::Player;
};