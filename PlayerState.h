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
    } 
    
    //==========================================================
    // Źródło odtwarzania
    //==========================================================

    //----------------------------------------------------------
    // Aktualnie wybrane źródło dźwięku.
    //
    // Przykłady:
    // Spotify
    // AirPlay
    // Bluetooth
    // USB
    // NAS
    //----------------------------------------------------------

    const char* source;


    //==========================================================
    // Informacje o utworze
    //==========================================================

    //----------------------------------------------------------
    // Nazwa wykonawcy.
    //----------------------------------------------------------

    const char* artist;

    //----------------------------------------------------------
    // Tytuł aktualnie odtwarzanego utworu.
    //----------------------------------------------------------

    const char* title;


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