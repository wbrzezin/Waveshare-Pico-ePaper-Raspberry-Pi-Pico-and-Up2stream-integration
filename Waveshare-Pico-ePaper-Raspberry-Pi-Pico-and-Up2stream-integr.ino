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


//==============================================================
// Utworzenie obiektu odpowiedzialnego za obsługę wyświetlacza.
//==============================================================

Display display;


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
    // Wyświetlenie przykładowego ekranu odtwarzacza.
    //
    // Docelowo dane będą pobierane z modułu Arylic UP2Stream.
    //----------------------------------------------------------

    display.showPlayer(
        "Spotify",
        "Dire Straits",
        "Money For Nothing");
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
}