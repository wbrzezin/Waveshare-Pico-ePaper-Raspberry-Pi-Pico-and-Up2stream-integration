//#pragma once

//==============================================================
// Projekt : UP2Stream Display
// Plik    : ScreenManager.h
// Autor   : Waldemar Brzeziński
//
// Opis:                                                        // Description:
// Zarządzanie aktualnie wyświetlanym ekranem.                 // Management of the currently displayed screen.
//
// W kolejnych etapach klasa będzie odpowiedzialna za           // In subsequent stages, the class will be responsible for
// automatyczne przełączanie pomiędzy ekranem odtwarzacza      // automatically switching between the player screen
// oraz ekranem bezczynności.                                  // and the idle screen.
//==============================================================

//--------------------------------------------------------------//-----------------------------------------------//
// Dostępne ekrany aplikacji.                                  // Available application screens.
//--------------------------------------------------------------//-----------------------------------------------//

enum class Screen
{
    //----------------------------------------------------------//-----------------------------------------------//
    // Ekran odtwarzacza.                                      // Player screen.
//----------------------------------------------------------//-----------------------------------------------//

    Player,

    //----------------------------------------------------------//-----------------------------------------------//
    // Ekran bezczynności.                                    // Idle screen.
//----------------------------------------------------------//-----------------------------------------------//

    Idle
};

//==============================================================
// Klasa ScreenManager                                         // ScreenManager class
//==============================================================

class ScreenManager
{
public:

    //----------------------------------------------------------//-----------------------------------------------//
    // Ustawienie aktywnego ekranu.                             // Set the active screen.
//--------------------------------------------------------------//-----------------------------------------------//

    void setScreen(Screen screen);

    //----------------------------------------------------------//-----------------------------------------------//
    // Odczyt aktualnego ekranu.                               // Read the current screen.
//--------------------------------------------------------------//-----------------------------------------------//

    Screen getScreen() const;

private:

    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualnie wyświetlany ekran.                            // Currently displayed screen.
//--------------------------------------------------------------//-----------------------------------------------//

    Screen currentScreen = Screen::Player;
};