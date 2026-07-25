
//==============================================================
// Projekt : UP2Stream Display
// Plik    : Display.h
// Autor   : Waldemar Brzeziński
// Opis    : Deklaracja klasy Display.
//==============================================================


#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include "PlayerState.h"


//--------------------------------------------------------------
// Klasa Display
//
// Odpowiada za obsługę wyświetlacza e-paper oraz rysowanie
// interfejsu użytkownika odtwarzacza UP2Stream.
//
// Zadania klasy:
// - inicjalizacja wyświetlacza,
// - wyświetlenie ekranu startowego,
// - rysowanie głównego ekranu odtwarzacza,
// - aktualizacja elementów dynamicznych,
// - obsługa przewijania długich nazw utworów i wykonawców.
//--------------------------------------------------------------
class Display
{
public:

    //==========================================================
    // Funkcje publiczne
    // Wywoływane przez główny program.
    //==========================================================

    //----------------------------------------------------------
    // Inicjalizacja wyświetlacza.
    //
    // Funkcja konfiguruje interfejs SPI, uruchamia sterownik
    // wyświetlacza oraz wyświetla ekran startowy.
    //
    // Zwraca:
    // true - inicjalizacja zakończyła się powodzeniem.
    //----------------------------------------------------------
    bool begin();

    //----------------------------------------------------------
    // Wyświetlenie ekranu startowego.
    //
    // Funkcja wykonywana jednorazowo podczas uruchamiania
    // urządzenia.
    //----------------------------------------------------------
    void splash();

   //----------------------------------------------------------
   // Wyświetlenie głównego ekranu odtwarzacza.
   //
   // Parametr:
   // player - komplet informacji o aktualnym stanie
   //          odtwarzacza.
   //----------------------------------------------------------

void showPlayer(const PlayerState& player);

    //----------------------------------------------------------
    // Aktualizacja elementów dynamicznych.
    //
    // Docelowo funkcja będzie odpowiedzialna za płynne
    // przewijanie długich nazw utworów i wykonawców oraz
    // odświeżanie wybranych fragmentów ekranu.
    //----------------------------------------------------------
    void scrollText();

private:

    //==========================================================
    // Funkcje odpowiedzialne za rysowanie interfejsu użytkownika
    //==========================================================

    //==========================================================
    // Funkcja drawPlayerScreen()
    //
    // Rysuje kompletny ekran odtwarzacza.
    //
    // Funkcja jest wykorzystywana podczas pełnego odświeżania
    // wyświetlacza e-paper.
    //
    // Parametry:
    //
    // player
    //      Aktualny stan odtwarzacza.
    //
    //==========================================================
    void drawPlayerScreen(const PlayerState& player);

    //----------------------------------------------------------
    // Rysowanie nagłówka z nazwą źródła dźwięku.
    //----------------------------------------------------------
    void drawHeader(const char* source);

    //----------------------------------------------------------
    // Rysowanie poziomej linii oddzielającej sekcje ekranu.
    //----------------------------------------------------------
    void drawSeparator(int y);

    //----------------------------------------------------------
    // Rysowanie tytułu aktualnie odtwarzanego utworu.
    //----------------------------------------------------------
    void drawTitle(const char* title);

    //----------------------------------------------------------
    // Rysowanie nazwy wykonawcy.
    //----------------------------------------------------------
    void drawArtist(const char* artist);

    //----------------------------------------------------------
    // Rysowanie paska postępu odtwarzania.
    //
    // currentTime
    //      Aktualny czas odtwarzania.
    //
    // totalTime
    //      Całkowity czas utworu.
    //
    // progress
    //      Postęp odtwarzania (0...100%).
    //----------------------------------------------------------
    void drawPlaybackBar(const char* currentTime,
                         const char* totalTime,
                         int progress);

    //----------------------------------------------------------
    // Rysowanie aktualnego poziomu głośności.
    //
    // volume
    //      Aktualny poziom głośności (0...100%).
    //----------------------------------------------------------
    void drawVolume(int volume);


    //==========================================================
    // Bufor aktualnie wyświetlanych danych
    //
    // Zmienne przechowują wartości, które zostały ostatnio
    // narysowane na ekranie. Dzięki temu możliwe będzie
    // wykonywanie częściowego odświeżania wyświetlacza
    // wyłącznie dla elementów, które uległy zmianie.
    //==========================================================

    //----------------------------------------------------------
    // Aktualnie wyświetlane źródło dźwięku.
    //----------------------------------------------------------
    const char* currentSource = nullptr;

    //----------------------------------------------------------
    // Aktualnie wyświetlany poziom głośności.
    //----------------------------------------------------------
    int currentVolume = -1;

    //----------------------------------------------------------
    // Aktualnie wyświetlany postęp odtwarzania.
    //----------------------------------------------------------
    int currentProgress = -1;


    //==========================================================
    // Parametry przewijania tekstu
    //==========================================================

    //----------------------------------------------------------
    // Aktualne przesunięcie tytułu utworu.
    //
    // Wartość określa liczbę pikseli, o jaką tekst został
    // przesunięty podczas przewijania.
    //----------------------------------------------------------
    int titleOffset = 0;

    //----------------------------------------------------------
    // Aktualne przesunięcie nazwy wykonawcy.
    //
    // Wartość określa liczbę pikseli, o jaką tekst został
    // przesunięty podczas przewijania.
    //----------------------------------------------------------
    int artistOffset = 0;

};

#endif