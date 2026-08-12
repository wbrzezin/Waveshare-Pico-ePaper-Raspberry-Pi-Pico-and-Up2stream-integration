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
// Informacja o synchronizacji RTC.
//
// RTC synchronizujemy tylko raz po uruchomieniu programu,
// po otrzymaniu pierwszej poprawnej odpowiedzi TME.
//
// Nie wolno ustawiać RTC w każdym przebiegu loop(), ponieważ
// zegar przestałby samodzielnie odmierzać czas.
//==============================================================

bool rtcSynchronized = false;

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



    
    //----------------------------------------------------------
    // Krótkie opóźnienie umożliwiające obejrzenie ekranu
    // startowego.
    //----------------------------------------------------------

    delay(3000);

    up2stream.query("TME;");

//--------------------------------------------------------------
// Pobranie aktualnego źródła odtwarzania.
//
// Up2Stream odpowie komunikatem:
//
//     SRC:NET;
//     SRC:BT;
//     SRC:LINE-IN;
//     itd.
//
// Odpowiedź zostanie odebrana i przetworzona podczas
// kolejnego wywołania up2stream.update().
//--------------------------------------------------------------

up2stream.query("SRC;");

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


//==============================================================
// Wyjście z trybu standby.
//
// Po SYS:ON ponownie pytamy Up2Stream o aktualne źródło.
//
// Nie pytamy o VND - Up2Stream wysyła VND spontanicznie.
//==============================================================

if ((changes & ChangeFlags::Standby)
    != ChangeFlags::None &&
    !currentState.standby)
{
    //----------------------------------------------------------
    // Pobranie aktualnego źródła.
    //----------------------------------------------------------

    up2stream.query("SRC;");
}


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
// Synchronizacja RTC.
//
// up2stream.update() właśnie odebrało dane z UART.
//
// Jeżeli odpowiedź TME została odebrana i poprawnie
// sparsowana, możemy przeliczyć ją na czas polski
// i ustawić zegar RTC Raspberry Pi Pico.
//
// RTC synchronizujemy tylko raz.
//
//==============================================================

if (!rtcSynchronized)
{
    const Up2StreamTime& up2streamTime =
        up2stream.getTime();

    //----------------------------------------------------------
    // Sprawdzenie, czy otrzymaliśmy poprawną odpowiedź TME.
    //----------------------------------------------------------

    if (up2streamTime.valid)
    {
        //------------------------------------------------------
        // Przeliczenie czasu UTC + offset + DST
        // na aktualny czas polski.
        //------------------------------------------------------

        PolishTime polishTime =
            convertToPolishTime(
                up2streamTime);

        //------------------------------------------------------
        // Jeżeli wynik jest poprawny, ustaw RTC.
        //------------------------------------------------------

        if (polishTime.valid)
        {
            display.setRTC(
                polishTime);

            //--------------------------------------------------
            // RTC został zsynchronizowany.
            //
            // Nie ustawiamy go ponownie przy kolejnych
            // odpowiedziach UART.
            //--------------------------------------------------

            rtcSynchronized = true;
        }
    }
}

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