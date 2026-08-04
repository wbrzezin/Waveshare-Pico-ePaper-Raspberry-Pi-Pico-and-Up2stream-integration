//==============================================================
// Projekt : UP2Stream Display
// Plik    : Up2StreamClient.h
//
// Opis:
//
// Klasa odpowiedzialna za komunikację UART z modułem
// Arylic Up2Stream.
//
// Zadania klasy:
//
// • odbiór danych z UART,
// • składanie komunikatów,
// • analiza protokołu,
// • aktualizacja struktury PlayerState,
// • zgłaszanie zmian do modułu Display.
//
// Klasa nie rysuje niczego na ekranie.
//==============================================================

#ifndef UP2STREAMCLIENT_H
#define UP2STREAMCLIENT_H

//--------------------------------------------------------------
// Biblioteki.
//--------------------------------------------------------------

#include <Arduino.h>

#include "PlayerState.h"

#include "ChangeFlags.h"


//==============================================================
// Klasa Up2StreamClient
//==============================================================

class Up2StreamClient
{
public:

    //----------------------------------------------------------
    // Konstruktor.
    //----------------------------------------------------------

    Up2StreamClient();


    //----------------------------------------------------------
    // Inicjalizacja komunikacji UART.
    //----------------------------------------------------------

    void begin(HardwareSerial& serial);


    //----------------------------------------------------------
    // Aktualizacja.
    //
    // Funkcja odczytuje dane z UART i zwraca informacje
    // o zmianach wykrytych w stanie odtwarzacza.
    //----------------------------------------------------------

    ChangeFlags update(PlayerState& player);


private:

    //----------------------------------------------------------
    // Wskaźnik do portu UART.
    //----------------------------------------------------------

    HardwareSerial* uart;

        //----------------------------------------------------------
    // Bufor odbierający pojedynczy komunikat UART.
    //
    // Przykład zawartości:
    //
    // PLA:1;
    // VOL:35;
    // TIT:Hurt;
    //----------------------------------------------------------

    static const uint16_t RX_BUFFER_SIZE = 256;

    char rxBuffer[RX_BUFFER_SIZE];


    //----------------------------------------------------------
    // Aktualna pozycja zapisu w buforze.
    //----------------------------------------------------------

    uint16_t rxPosition;

        //----------------------------------------------------------
    // Analiza pojedynczego komunikatu odebranego z UART.
    //
    // Parametry:
    //   message - kompletny komunikat zakończony ';'
    //   player  - aktualny stan odtwarzacza
    //
    // Zwraca:
    //   ChangeFlags informujące, które elementy interfejsu
    //   wymagają odświeżenia.
    //----------------------------------------------------------

    ChangeFlags processMessage(
        const char* message,
        PlayerState& player);

};

#endif