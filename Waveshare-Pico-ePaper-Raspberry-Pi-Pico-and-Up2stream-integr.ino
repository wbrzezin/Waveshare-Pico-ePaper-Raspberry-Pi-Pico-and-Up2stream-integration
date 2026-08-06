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
#include "Up2StreamClient.h"
#include "HardwareConfig.h"

//==============================================================
// Utworzenie obiektu odpowiedzialnego za obsługę wyświetlacza.
//==============================================================

Display display;

//--------------------------------------------------------------
// Komunikacja z modułem Up2Stream.
//--------------------------------------------------------------

Up2StreamClient up2stream;

//--------------------------------------------------------------
// Źródło danych odtwarzacza.
//
// true  - dane testowe
// false - komunikacja z modułem Up2Stream
//--------------------------------------------------------------

constexpr bool USE_TEST_DATA = true;

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
// Wczytanie przykładowych danych.
//
// Funkcja wykorzystywana podczas projektowania interfejsu
// użytkownika bez podłączonego modułu Up2Stream.
//==============================================================

void loadTestData(PlayerState& state)
{
    state.source = "Spotify";

    state.artist = "Zażółć gęślą jaźń";

    state.title =
        "Money For Nothing - Dire Straits - Brothers In Arms - Remastered 2025";

    state.currentTime = "02:15";

    state.totalTime = "08:26";

    state.progress = 35;

    state.volume = 38;

    state.playing = true;
}

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
    // Konfiguracja portu UART wykorzystywanego do komunikacji
    // z modułem Up2Stream.
    //----------------------------------------------------------


    UP2STREAM_SERIAL.setTX(
        UP2STREAM_UART_TX_PIN);

    UP2STREAM_SERIAL.setRX(
        UP2STREAM_UART_RX_PIN);

    UP2STREAM_SERIAL.begin(115200);

    //----------------------------------------------------------
    // Inicjalizacja klienta Up2Stream.
    //----------------------------------------------------------

    up2stream.begin(
        UP2STREAM_SERIAL);


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

loadTestData(currentState);


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
    // Odczyt danych z modułu Up2Stream.
    //----------------------------------------------------------

    ChangeFlags changes =
        up2stream.update(currentState);

    //----------------------------------------------------------
    // Aktualizacja wyświetlacza.
    //----------------------------------------------------------

    display.update(
        currentState,
        changes);

    //----------------------------------------------------------
    // Ograniczenie częstotliwości odświeżania.
    //----------------------------------------------------------

    delay(40);
}