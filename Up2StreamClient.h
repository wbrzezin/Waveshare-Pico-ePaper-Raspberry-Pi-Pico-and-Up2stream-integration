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
// Czas odebrany z modułu Up2Stream.
//
// Dane pochodzą z odpowiedzi:
//
// TME:YYYY-MM-DD HH:MM:SS (+offset);
//==============================================================

struct Up2StreamTime
{
    int year = 0;
    int month = 0;
    int day = 0;

    int hour = 0;
    int minute = 0;
    int second = 0;

    int utcOffset = 0;

    bool valid = false;
};

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
// Wysłanie zapytania do modułu Up2Stream.
//
// Zapytanie powinno być zakończone znakiem ';'.
//
// Przykłady:
//     query("TME;");
//     query("SRC;");
//     query("STA;");
//----------------------------------------------------------

void query(const char* command);

    //----------------------------------------------------------
    // Aktualizacja.
    //
    // Funkcja odczytuje dane z UART i zwraca informacje
    // o zmianach wykrytych w stanie odtwarzacza.
    //----------------------------------------------------------

    ChangeFlags update(PlayerState& player);

//----------------------------------------------------------
// Zwraca ostatni poprawnie odebrany czas TME.
//----------------------------------------------------------

const Up2StreamTime& getTime() const;

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
    // Informacja o zmianie utworu.
    //
    // Po odebraniu TIT poprzednia wartość totalTime
    // przestaje być wiarygodna dla nowego utworu.
    //
    // Czekamy wtedy na pierwszą komendę ELP,
    // która poda aktualny czas oraz całkowity czas
    // nowego utworu.
    //----------------------------------------------------------

    bool waitingForTrackELP;

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

//----------------------------------------------------------
// Ostatni poprawnie odebrany czas TME.
//----------------------------------------------------------

Up2StreamTime up2streamTime;    

        

};

#endif