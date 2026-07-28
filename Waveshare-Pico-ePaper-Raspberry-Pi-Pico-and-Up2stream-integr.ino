//==============================================================
// Projekt: UP2Stream Display
//
// Główny program sterujący wyświetlaczem e-paper dla modułu
// Arylic UP2Stream.
//
// Zadania programu:
// - inicjalizacja portu szeregowego,
// - inicjalizacja wyświetlacza,
// - wyświetlenie ekranu odtwarzacza,
// - cykliczna aktualizacja elementów dynamicznych.
//==============================================================

#include "Display.h"
#include "StateComparer.h"
#include "UTF8Print.h"

//==============================================================
// Utworzenie obiektu odpowiedzialnego za obsługę wyświetlacza.
//==============================================================

Display display;

//==============================================================
// Bufory stanu odtwarzacza.
//
// previousState
//      Stan odtwarzacza wyświetlony podczas poprzedniego
//      odświeżenia.
//
// currentState
//      Aktualny stan odebrany z modułu UP2Stream.
//
//==============================================================

PlayerState previousState;
PlayerState currentState;


//==============================================================
// Funkcja setup()
//
// Wykonywana jednorazowo po uruchomieniu mikrokontrolera.
//==============================================================

void setup()
{


    
    //----------------------------------------------------------
    // Uruchomienie portu szeregowego.
    // Wykorzystywany podczas uruchamiania oraz debugowania.
    //----------------------------------------------------------

    Serial.begin(115200);

    while (!Serial)
        delay(10);

    Serial.println("Start");

    //----------------------------------------------------------
    // Inicjalizacja wyświetlacza.
    //----------------------------------------------------------

    display.begin();

    //----------------------------------------------------------
    // Krótkie opóźnienie umożliwiające obejrzenie ekranu
    // startowego.
    //----------------------------------------------------------

    delay(3000);

//----------------------------------------------------------
// Przygotowanie przykładowych danych.
//
// Docelowo informacje będą pobierane z modułu Arylic
// UP2Stream.
//----------------------------------------------------------

currentState.source      = "Spotify";
currentState.artist      = "Zażółć gęślą jaźń";
currentState.title       = "Money For Nothing - Dire Straits - Brothers In Arms - Remastered 2025";

currentState.currentTime = "02:15";
currentState.totalTime   = "08:26";

currentState.progress    = 35;
currentState.volume      = 38;

currentState.playing     = true;


//==========================================================
// Porównanie poprzedniego oraz aktualnego stanu odtwarzacza.
//
// Funkcja compare() zwraca zestaw flag określających,
// które elementy stanu odtwarzacza uległy zmianie.
//
//==========================================================

ChangeFlags changes =
    StateComparer::compare(previousState, currentState);


//==========================================================
// Wyświetlenie aktualnego stanu odtwarzacza.
//
//==========================================================

display.update(currentState, changes);


//==========================================================
// Zapamiętanie aktualnego stanu.
//
// Podczas kolejnego odświeżenia będzie on traktowany jako
// stan poprzedni.
//
//==========================================================

previousState = currentState;


}


//==============================================================
// Funkcja loop()
//
// Główna pętla programu wykonywana w sposób ciągły.
//
// Obecnie pozostaje pusta.
// Docelowo będzie odpowiedzialna za:
//
// - odczyt danych z modułu UP2Stream,
// - aktualizację czasu odtwarzania,
// - przewijanie długich nazw,
// - odświeżanie paska postępu,
// - obsługę przycisków,
// - aktualizację ikon i informacji o stanie urządzenia.
//==============================================================

void loop()
{
    //----------------------------------------------------------
    // Aktualizacja przewijania.
    //----------------------------------------------------------

    ChangeFlags changes = ChangeFlags::None;

    display.update(currentState, changes);

    delay(40);
}