//==============================================================
// Projekt : UP2Stream Display
// Plik    : PlayerState.h
//
// Opis:
//
// Definicja struktury PlayerState.
//
// Struktura przechowuje kompletny stan odtwarzacza
// przekazywany do modułu odpowiedzialnego za rysowanie
// interfejsu użytkownika.
//
// Dzięki wydzieleniu jej do osobnego pliku może być
// wykorzystywana przez wiele modułów programu:
//
//  • komunikację z modułem Arylic,
//  • moduł wyświetlacza,
//  • serwer WWW,
//  • MQTT,
//  • zapis ustawień.
//
// Dzięki temu wszystkie moduły korzystają z jednego,
// wspólnego opisu aktualnego stanu odtwarzacza.
//==============================================================

#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H

//--------------------------------------------------------------
// Klasa String wykorzystywana do przechowywania informacji
// tekstowych odbieranych z modułu Up2Stream.
//
// Dzięki temu parser UART może bezpośrednio przypisywać
// odebrane wartości do struktury PlayerState.
//--------------------------------------------------------------

#include <Arduino.h>


//==============================================================
// Struktura PlayerState
//
// Zawiera komplet informacji opisujących aktualny stan
// odtwarzacza.
//
// W przyszłości struktura będzie uzupełniana o kolejne
// pola, np.:
//
// • nazwę albumu,
// • okładkę,
// • bitrate,
// • częstotliwość próbkowania,
// • rozdzielczość,
// • stan Wi-Fi,
// • informacje o Bluetooth,
// • poziom głośności,
// • stan odtwarzania.
//
// Dzięki temu funkcje odpowiedzialne za rysowanie ekranu
// otrzymują tylko jeden parametr.
//==============================================================

struct PlayerState
{
    
    //==========================================================
    // Konstruktor domyślny
    //
    // Inicjalizuje wszystkie pola struktury wartościami
    // domyślnymi.
    //
    // Dzięki temu obiekt PlayerState jest zawsze w poprawnym
    // stanie i może być bezpiecznie wykorzystywany do
    // porównywania oraz aktualizacji interfejsu użytkownika.
    //==========================================================

    PlayerState()
    {
        source      = "";
        artist      = "";
        title       = "";

        currentTime = "00:00";
        totalTime   = "00:00";

        progress    = 0;
        volume      = 0;

        playing     = false;

        album = "";

        vendor = "";

        elapsedMs = 0;

        totalMs = 0;

        playlistIndex = 0;

        playlistCount = 0;

        muted = false;

        audioActive = false;
    } 
    
    //==========================================================
    // Źródło odtwarzania
    //==========================================================

    //----------------------------------------------------------
    // Aktualnie wybrane źródło dźwięku.
    //
    // Przykłady:
    //
    // NET
    // LINE-IN
    // USB
    // BT
    //----------------------------------------------------------

    String source;


//----------------------------------------------------------
// Nazwa wykonawcy.
//----------------------------------------------------------

String artist;


//----------------------------------------------------------
// Tytuł aktualnie odtwarzanego utworu.
//----------------------------------------------------------

String title;

    //==========================================================
    // Czas odtwarzania
    //==========================================================

    //----------------------------------------------------------
    // Aktualny czas odtwarzania.
    //
    // Przykład:
    // 02:15
    //----------------------------------------------------------

    const char* currentTime;

    //----------------------------------------------------------
    // Całkowity czas trwania utworu.
    //
    // Przykład:
    // 08:26
    //----------------------------------------------------------

    const char* totalTime;


    //==========================================================
    // Postęp odtwarzania
    //==========================================================

    //----------------------------------------------------------
    // Procentowy postęp odtwarzania.
    //
    // Zakres:
    // 0...100
    //----------------------------------------------------------

    int progress;


    //==========================================================
    // Głośność
    //==========================================================

    //----------------------------------------------------------
    // Aktualny poziom głośności.
    //
    // Zakres:
    // 0...100
    //----------------------------------------------------------

    int volume;
//==========================================================
// Informacje rozszerzone
//==========================================================

//----------------------------------------------------------
// Nazwa albumu.
//
// Przesyłana przez moduł Up2Stream komunikatem:
//
// ALB:...
//----------------------------------------------------------

String album;

//----------------------------------------------------------
// Nazwa dostawcy usługi.
//
// Przykłady:
//
// Spotify
// Tidal
// Radio
// unknown
//
// Komunikat:
//
// VND:...
//----------------------------------------------------------

String vendor;


//==========================================================
// Dane źródłowe odtwarzania
//==========================================================

//----------------------------------------------------------
// Aktualna pozycja odtwarzania.
//
// Jednostka:
//
// milisekundy
//
// Komunikat:
//
// ELP:12345/45678;
//----------------------------------------------------------

uint32_t elapsedMs;

//----------------------------------------------------------
// Całkowity czas utworu.
//
// Jednostka:
//
// milisekundy
//
// Komunikat:
//
// ELP:12345/45678;
//----------------------------------------------------------

uint32_t totalMs;


//==========================================================
// Informacje o playliście
//==========================================================

//----------------------------------------------------------
// Numer aktualnego utworu.
//
// Komunikat:
//
// PLI:7/20;
//----------------------------------------------------------

uint16_t playlistIndex;

//----------------------------------------------------------
// Liczba utworów.
//
// Komunikat:
//
// PLI:7/20;
//----------------------------------------------------------

uint16_t playlistCount;


//==========================================================
// Dodatkowe stany odtwarzacza
//==========================================================

//----------------------------------------------------------
// Wyciszenie.
//
// true  - Mute
// false - normalne odtwarzanie
//
// Komunikat:
//
// MUT:0
// MUT:1
//----------------------------------------------------------

bool muted;


//----------------------------------------------------------
// Informacja o aktywnym strumieniu audio.
//
// false - brak strumienia
// true  - audio aktywne
//
// Komunikat:
//
// AUD:0
// AUD:1
//----------------------------------------------------------

bool audioActive;

    //==========================================================
    // Stan odtwarzacza
    //==========================================================

    //----------------------------------------------------------
    // Informacja o stanie odtwarzania.
    //
    // true  - odtwarzanie
    // false - pauza
    //----------------------------------------------------------

    bool playing;
};

#endif