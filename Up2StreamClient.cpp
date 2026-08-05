//==============================================================
// Projekt : UP2Stream Display
// Plik    : Up2StreamClient.cpp
//
// Opis:
//
// Implementacja klasy odpowiedzialnej za komunikację UART
// z modułem Arylic Up2Stream.
//
// Na obecnym etapie implementowane są jedynie:
//
// • konstruktor,
// • inicjalizacja,
// • szkielet funkcji update().
//
// Parser komunikatów zostanie dodany w kolejnych etapach.
//==============================================================

#include "Up2StreamClient.h"


//==============================================================
// Konstruktor.
//
// Zeruje wskaźnik do portu UART.
//==============================================================

Up2StreamClient::Up2StreamClient()
{
    //----------------------------------------------------------
    // Port UART nie został jeszcze przypisany.
    //----------------------------------------------------------

    uart = nullptr;

    //----------------------------------------------------------
    // Początek bufora odbiorczego.
    //----------------------------------------------------------

    rxPosition = 0;

    rxBuffer[0] = '\0';
}

//==============================================================
// Inicjalizacja.
//
// Zapamiętuje port UART wykorzystywany do komunikacji
// z modułem Up2Stream.
//
// Sama konfiguracja prędkości transmisji będzie wykonywana
// w programie głównym.
//==============================================================

void Up2StreamClient::begin(HardwareSerial& serial)
{
    uart = &serial;
}


//==============================================================
// Aktualizacja.
//
// Docelowo funkcja będzie:
//
// • odbierała dane z UART,
// • analizowała komunikaty,
// • aktualizowała PlayerState,
// • zwracała ChangeFlags.
//
// Na obecnym etapie nie wykonuje jeszcze żadnych działań.
//==============================================================

//==============================================================
// Aktualizacja.
//
// Odczytuje wszystkie znaki oczekujące w buforze UART.
//
// Na obecnym etapie:
//
// • odbiera znaki,
// • zapisuje je do bufora,
// • wykrywa koniec komunikatu (';').
//
// Analiza odebranego komunikatu zostanie dodana
// w następnym kroku.
//==============================================================

ChangeFlags Up2StreamClient::update(PlayerState& player)
{
    (void)player;

    //----------------------------------------------------------
    // Jeżeli UART nie został jeszcze zainicjalizowany,
    // nie wykonujemy żadnych działań.
    //----------------------------------------------------------

    if (uart == nullptr)
    {
        return ChangeFlags::None;
    }

    //----------------------------------------------------------
    // Odczytaj wszystkie dostępne znaki.
    //----------------------------------------------------------

    while (uart->available())
    {
        //------------------------------------------------------
        // Pobierz jeden znak z UART.
        //------------------------------------------------------

        char c = uart->read();

        //------------------------------------------------------
        // Diagnostyka UART.
        //
        // Wszystkie odebrane znaki są kopiowane na port
        // debugowania. Dzięki temu można sprawdzić, czy Pico
        // odbiera dokładnie te same dane, które wcześniej były
        // widoczne w PuTTY.
        //------------------------------------------------------

        Serial.write(c);

        //------------------------------------------------------
        // Zabezpieczenie przed przepełnieniem bufora.
        //------------------------------------------------------

        if (rxPosition >= RX_BUFFER_SIZE - 1)
        {
            rxPosition = 0;
        }

        //------------------------------------------------------
        // Dopisz znak do bufora.
        //------------------------------------------------------

        rxBuffer[rxPosition++] = c;

        //------------------------------------------------------
        // Zawsze zakończ napis znakiem '\0'.
        //------------------------------------------------------

        rxBuffer[rxPosition] = '\0';

        //------------------------------------------------------
        // Czy odebrano koniec komunikatu?
        //------------------------------------------------------

        if (c == ';')
        {
            //--------------------------------------------------
// Przetwórz kompletny komunikat.
//--------------------------------------------------

ChangeFlags changes =
    processMessage(
        rxBuffer,
        player);

//--------------------------------------------------
// Jeżeli parser wykrył zmianę,
// zwróć ją do programu głównego.
//--------------------------------------------------

if (changes != ChangeFlags::None)
{
    rxPosition = 0;
    rxBuffer[0] = '\0';

    return changes;
}
            //--------------------------------------------------
            // Wyczyść bufor.
            //--------------------------------------------------

            rxPosition = 0;

            rxBuffer[0] = '\0';
        }
    }

    return ChangeFlags::None;
}

//==============================================================
// Analiza pojedynczego komunikatu.
//
// Na obecnym etapie obsługiwana jest wyłącznie komenda PLA.
//
// Przykłady:
//
// PLA:0;
// PLA:1;
//==============================================================

ChangeFlags Up2StreamClient::processMessage(
    const char* message,
    PlayerState& player)
{
    //----------------------------------------------------------
    // Komenda PLAY / PAUSE.
    //----------------------------------------------------------

    if (strncmp(message, "PLA:", 4) == 0)
    {
        //------------------------------------------------------
        // Odczytaj stan odtwarzania.
        //------------------------------------------------------

        player.playing = (message[4] == '1');

        //------------------------------------------------------
        // Poinformuj wyświetlacz, że zmienił się stan
        // odtwarzania.
        //------------------------------------------------------

        return ChangeFlags::PlayState;
    }

    //----------------------------------------------------------
    // Komunikat nie został jeszcze obsłużony.
    //----------------------------------------------------------

    return ChangeFlags::None;
}