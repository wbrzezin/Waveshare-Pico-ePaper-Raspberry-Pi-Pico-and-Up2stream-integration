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
#include "PolishTime.h"

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
// Flaga testu czasu.
//
// Test wykonujemy tylko raz, po otrzymaniu pierwszej poprawnej
// odpowiedzi TME.
//==============================================================

bool timeTestDone = false;

//==============================================================
// Wczytanie przykładowych danych.
//
// Funkcja wykorzystywana podczas projektowania interfejsu
// użytkownika bez podłączonego modułu Up2Stream.
//==============================================================

void loadTestData(PlayerState& state)
{
    Serial.println("!!! loadTestData() !!!");
    
    state.source = "--";

    state.artist = "...";

    state.title =
        "...";

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

    Serial.println(">>> TX TEST");

UP2STREAM_SERIAL.print("TME;");
UP2STREAM_SERIAL.flush();

Serial.println(">>> TX TEST END");

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

//----------------------------------------------------------
// Zwiększenie sprzętowego bufora odbiorczego UART.
//
// Domyślny FIFO UART w Arduino-Pico ma 32 bajty.
// Zwiększamy go do 128 bajtów, aby ograniczyć ryzyko
// utraty znaków podczas odbioru dłuższych komunikatów,
// szczególnie podczas pracy wyświetlacza e-paper.
//
// UWAGA:
// setFIFOSize() musi zostać wywołane przed begin().
//----------------------------------------------------------

UP2STREAM_SERIAL.setFIFOSize(128);

UP2STREAM_SERIAL.begin(115200);

    //----------------------------------------------------------
    // Inicjalizacja klienta Up2Stream.
    //----------------------------------------------------------

    up2stream.begin(
        UP2STREAM_SERIAL);

//==============================================================
// Pobranie aktualnego czasu z UP2Stream.
//
// Jest to tymczasowe zapytanie testowe.
// W docelowej wersji będzie wysyłane przez mechanizm
// synchronizacji zegara.
//==============================================================

delay(1000);

up2stream.query("TME;");

    
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

    //==============================================================
// Test przeliczenia czasu UP2Stream na czas polski.
//
// Test wykonywany jest tylko raz - po otrzymaniu poprawnej
// odpowiedzi TME.
//
// Na tym etapie wynik jest wyłącznie wypisywany przez UART.
// RTC oraz ekran bezczynności nie są jeszcze wykorzystywane.
//==============================================================

if (!timeTestDone)
{
    const Up2StreamTime& t =
        up2stream.getTime();

    //----------------------------------------------------------
    // Sprawdzenie, czy otrzymaliśmy poprawną odpowiedź TME.
    //----------------------------------------------------------

    if (t.valid)
    {
        //------------------------------------------------------
        // Przeliczenie czasu na czas polski.
        //------------------------------------------------------

        PolishTime polish =
            convertToPolishTime(t);

        //------------------------------------------------------
        // Diagnostyka wyniku.
        //------------------------------------------------------

        Serial.println(
            "POLISH TIME TEST");

        //------------------------------------------------------
        // Data.
        //------------------------------------------------------

        Serial.print("DATE = ");

        if (polish.day < 10)
            Serial.print('0');

        Serial.print(polish.day);

        Serial.print('.');

        if (polish.month < 10)
            Serial.print('0');

        Serial.print(polish.month);

        Serial.print('.');

        Serial.println(polish.year);

        //------------------------------------------------------
        // Godzina.
        //------------------------------------------------------

        Serial.print("TIME = ");

        if (polish.hour < 10)
            Serial.print('0');

        Serial.print(polish.hour);

        Serial.print(':');

        if (polish.minute < 10)
            Serial.print('0');

        Serial.print(polish.minute);

        Serial.print(':');

        if (polish.second < 10)
            Serial.print('0');

        Serial.println(polish.second);

        //------------------------------------------------------
        // Test został wykonany.
        //------------------------------------------------------

        timeTestDone = true;
    }
}

    //----------------------------------------------------------
    // Aktualizacja wyświetlacza.
    //----------------------------------------------------------

    display.update(
        currentState,
        changes);


//----------------------------------------------------------
// Aktualizacja ekranu zegara.
//
// Funkcja sama sprawdza, czy zmieniła się minuta.
// Jeżeli nie - natychmiast kończy działanie.
//
// Dzięki temu można ją wywoływać w każdej iteracji
// głównej pętli bez niepotrzebnego odświeżania e-paper.
//----------------------------------------------------------

display.updateIdle();     

    //----------------------------------------------------------
    // Ograniczenie częstotliwości odświeżania.
    //----------------------------------------------------------

    delay(40);
}