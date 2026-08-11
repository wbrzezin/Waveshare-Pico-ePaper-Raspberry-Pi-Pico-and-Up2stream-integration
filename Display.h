
//==============================================================
// Projekt : UP2Stream Display
// Plik    : Display.h
// Autor   : Waldemar Brzeziński
// Opis    : Deklaracja klasy Display.
//==============================================================


#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

//--------------------------------------------------------------
// Definicja struktury GFXfont wykorzystywanej przez Adafruit_GFXbezczynności
//--------------------------------------------------------------
#include <Adafruit_GFX.h>

#include "PlayerState.h"
#include "ChangeFlags.h"
#include "ScrollState.h"
#include "Icons.h"
#include "ScreenManager.h"
#include "PolishTime.h"

//--------------------------------------------------------------
// Tryb pracy interfejsu użytkownika.
//
// Player
//      Wyświetlany jest ekran odtwarzacza.
//
// Idle
//      Wyświetlany jest ekran bezczynności.
//--------------------------------------------------------------

enum class DisplayMode
{
    Player,
    Idle
};

//==============================================================
// Obszary ekranu
//==============================================================

static const int HEADER_TOP      = 0;
static const int HEADER_HEIGHT   = 24;

static const int TITLE_TOP       = 26;
static const int TITLE_HEIGHT    = 36;

static const int PROGRESS_TOP    = 92;
static const int PROGRESS_HEIGHT = 12;

static const int TIME_TOP        = 103;
static const int TIME_HEIGHT     = 16;

static const int FOOTER_TOP      = 118;
static const int FOOTER_HEIGHT   = 10;

//==============================================================
// Deklaracja struktury czasu polskiego.
//
// Pełna definicja znajduje się w PolishTime.h.
//==============================================================

struct PolishTime;


//--------------------------------------------------------------
// Klasa Display
//
// Odpowiada za obsługę wyświetlacza e-paper oraz rysowanie
// interfejsu użytkownika odtwarzacza UP2Stream.
//
// Zadania klasy:
// - inicjalizacja wyświetlacza,
// - wyświetlenie ekranu startowego,
// - rysowanie głównego ekranu odtwarzacza,
// - aktualizacja elementów dynamicznych,
// - obsługa przewijania długich nazw utworów i wykonawców.
//--------------------------------------------------------------
class Display
{
public:

    //==========================================================
    // Funkcje publiczne
    // Wywoływane przez główny program.
    //==========================================================

    //----------------------------------------------------------
    // Inicjalizacja wyświetlacza.
    //
    // Funkcja konfiguruje interfejs SPI, uruchamia sterownik
    // wyświetlacza oraz wyświetla ekran startowy.
    //
    // Zwraca:
    // true - inicjalizacja zakończyła się powodzeniem.
    //----------------------------------------------------------
    bool begin();

    //----------------------------------------------------------
    // Wyświetlenie ekranu startowego.
    //
    // Funkcja wykonywana jednorazowo podczas uruchamiania
    // urządzenia.
    //----------------------------------------------------------
    void splash();

   //----------------------------------------------------------
   // Wyświetlenie głównego ekranu odtwarzacza.
   //
   // Parametr:
   // player - komplet informacji o aktualnym stanie
   //          odtwarzacza.
   //----------------------------------------------------------

void showPlayer(const PlayerState& player);

//==============================================================
// Ustawienie zegara RTC.
//
// Parametr:
//
// time
//      Aktualny czas polski obliczony na podstawie odpowiedzi
//      TME z modułu UP2Stream.
//
//==============================================================

void setRTC(
    const PolishTime& time);

//--------------------------------------------------------------
// Wyświetlenie ekranu bezczynności.
//
// Funkcja wykonuje pełne odświeżenie wyświetlacza i rysuje
// ekran bezczynności.
//
//--------------------------------------------------------------

void showIdle();

    //--------------------------------------------------------------
    // Aktualizacja zegara na ekranie bezczynności.
    //
    // Ekran jest odświeżany tylko po zmianie minuty.
    //--------------------------------------------------------------
    void updateIdle();
    
    //----------------------------------------------------------
    // Aktualizacja zawartości wyświetlacza.
    //
    // Funkcja analizuje przekazane informacje o zmianach
    // i odświeża odpowiednie elementy interfejsu.
    //
    // Na obecnym etapie wykonywane jest jeszcze pełne
    // odświeżenie ekranu.
    //
    // Parametry:
    //
    // player
    //      Aktualny stan odtwarzacza.
    //
    // changes
    //      Flagi określające, które elementy uległy zmianie.
    //----------------------------------------------------------

    void update(const PlayerState& player,
                ChangeFlags changes);

    //----------------------------------------------------------
    // Aktualizacja elementów dynamicznych.
    //
    // Docelowo funkcja będzie odpowiedzialna za płynne
    // przewijanie długich nazw utworów i wykonawców oraz
    // odświeżanie wybranych fragmentów ekranu.
    //----------------------------------------------------------
    void scrollText();

private:

    //==========================================================
    // Funkcje odpowiedzialne za rysowanie interfejsu użytkownika
    //==========================================================

//==============================================================
// Funkcja refreshFull()
//
// Wykonuje pełne odświeżenie wyświetlacza.
//
//==============================================================

void refreshFull(const PlayerState& player);

//==============================================================
// Funkcja refreshPartial()
//
// Wykonuje częściowe odświeżenie wyświetlacza.
//
// Na razie funkcja pozostaje pusta.
// W kolejnych etapach będzie wykorzystywać setPartialWindow().
//
//==============================================================

void refreshPartial(const PlayerState& player,
                    ChangeFlags changes);


    //==========================================================
    // Funkcja drawPlayerScreen()
    //
    // Rysuje kompletny ekran odtwarzacza.
    //
    // Funkcja jest wykorzystywana podczas pełnego odświeżania
    // wyświetlacza e-paper.
    //
    // Parametry:
    //
    // player
    //      Aktualny stan odtwarzacza.
    //
    //==========================================================
    void drawPlayerScreen(const PlayerState& player);

    //--------------------------------------------------------------
    // Rysowanie ekranu bezczynności.
    //
    // Ekran wyświetlany po dłuższym okresie braku aktywności.
    // Docelowo będzie zawierał aktualny czas, datę oraz
    // konfigurowalny napis.
    //--------------------------------------------------------------

     void drawIdleScreen();

    //----------------------------------------------------------
    // Rysowanie nagłówka z nazwą źródła dźwięku.
    //----------------------------------------------------------
    void drawHeader(const char* source);

    //----------------------------------------------------------
    // Rysowanie poziomej linii oddzielającej sekcje ekranu.
    //----------------------------------------------------------
    void drawSeparator(int y);

    //----------------------------------------------------------
    // Rysowanie całego obszaru nagłówka.
    //----------------------------------------------------------
    void drawHeaderArea(const char* source);

        //----------------------------------------------------------
    // Rysowanie nazwy źródła dźwięku.
    //----------------------------------------------------------
    void drawSourceName(const char* source);

    
    //----------------------------------------------------------
    // Rysowanie informacji o źródle dźwięku.
    //----------------------------------------------------------
    void drawSourceInfo(const char* source);

   //==============================================================
   // Funkcja drawHeaderSeparator()
   //
   // Rysuje linię oddzielającą nagłówek od pozostałej części
   // ekranu.
   //
   // Parametry:
   //
   // brak
   //
   // Zwraca:
   //
   // nic
   //
   //==============================================================

    void drawHeaderSeparator();

    //--------------------------------------------------------------
    // Obliczenie szerokości tekstu.
    //
    // Funkcja zwraca szerokość napisu dla wskazanej czcionki.
    // Dzięki temu wynik nie zależy od aktualnie ustawionej
    // czcionki wyświetlacza.
    //--------------------------------------------------------------
    uint16_t measureTextWidth(
        const String& text,
        const GFXfont* font);

        //--------------------------------------------------------------
        // Obliczenie współrzędnej X zapewniającej wyśrodkowanie napisu.
        //
        // Parametry:
        //
        // text
        //      Tekst do wyświetlenia.
        //
        // font
        //      Czcionka użyta do obliczenia szerokości.
        //
        // areaX
        //      Lewa krawędź obszaru.
        //
        // areaWidth
        //      Szerokość obszaru.
        //
        // Zwraca:
        //
        // Współrzędną X, od której należy rozpocząć rysowanie tekstu.
        //--------------------------------------------------------------

        int calculateCenteredX(
            const String& text,
            const GFXfont* font,
            int areaX,
            int areaWidth);

        int scorePage(const String& page,
              const String& remaining,
              int areaWidth);


  //----------------------------------------------------------
  // Wyznaczenie fragmentu tekstu mieszczącego się
  // w dostępnym obszarze.
  //----------------------------------------------------------
 String getVisibleText(
    const String& text,
    ScrollState& scroll,
    int maxWidth);


//--------------------------------------------------------------
// Inicjalizacja przewijanego tekstu.
//
// Parametry:
//
// scroll
//      Struktura przechowująca stan przewijania.
//
// text
//      Tekst do wyświetlenia.
//
// areaWidth
//      Dostępna szerokość obszaru w pikselach.
//
// font
//      Czcionka wykorzystywana zarówno do pomiaru szerokości,
//      jak i późniejszego wyświetlenia tekstu.
//--------------------------------------------------------------
void initScroll(
    ScrollState& scroll,
    const char* text,
    int areaWidth,
    const GFXfont* font);

//==============================================================
// Funkcja updateScroll()
//
// Aktualizuje pozycję przewijanego tekstu.
//
//==============================================================

bool updateScroll(ScrollState& scroll);

//==============================================================
// Funkcja drawScrollingText()
//
// Rysuje tekst w zadanym obszarze.
//
//==============================================================

void drawScrollingText(const char* text,
                       ScrollState& scroll,
                       int x,
                       int y,
                       int width);




    //----------------------------------------------------------
    // Rysowanie tytułu aktualnie odtwarzanego utworu.
    //----------------------------------------------------------
    void drawTitle(const char* title);

    //----------------------------------------------------------
    // Rysowanie nazwy wykonawcy.
    //----------------------------------------------------------
    void drawArtist(const char* artist);

   //==============================================================
   // Funkcja drawTrackInfo()
   //
   // Rysuje informacje o aktualnie odtwarzanym utworze.
   //
   // Parametry:
   //
   // title
   //      Tytuł utworu.
   //
   // artist
   //      Wykonawca.
   //
   // Zwraca:
   //
   // nic
   //
   //==============================================================

   void drawTrackInfo(const char* title,
                   const char* artist);

   //----------------------------------------------------------
// Rysowanie paska postępu odtwarzania.
//
// Parametry:
//
// currentTime
//      Aktualny czas odtwarzania.
//
// totalTime
//      Całkowity czas utworu.
//
// progress
//      Postęp odtwarzania w procentach.
//
// playing
//      true  - odtwarzanie
//      false - pauza / zatrzymanie.
//----------------------------------------------------------

void drawPlaybackBar(
    const char* currentTime,
    const char* totalTime,
    int progress,
    bool playing);

    //----------------------------------------------------------
    // Rysowanie aktualnego poziomu głośności.
    //
    // volume
    //      Aktualny poziom głośności (0...100%).
    //----------------------------------------------------------
    void drawVolume(int volume);


    //==========================================================
    // Bufor aktualnie wyświetlanych danych
    //
    // Zmienne przechowują wartości, które zostały ostatnio
    // narysowane na ekranie. Dzięki temu możliwe będzie
    // wykonywanie częściowego odświeżania wyświetlacza
    // wyłącznie dla elementów, które uległy zmianie.
    //==========================================================

    //----------------------------------------------------------
    // Aktualnie wyświetlane źródło dźwięku.
    //----------------------------------------------------------
const char* currentSource = nullptr;

int currentVolume = -1;
int currentProgress = -1;

//--------------------------------------------------------------
// Stan przewijania tytułu.
//--------------------------------------------------------------

ScrollState titleScroll;

//--------------------------------------------------------------
// Stan przewijania wykonawcy.
//--------------------------------------------------------------

ScrollState artistScroll;

//--------------------------------------------------------------
// Stan ekranu bezczynności.
//
// false - wyświetlany jest ekran odtwarzacza
// true  - wyświetlany jest ekran zegara
//--------------------------------------------------------------

bool idleScreenActive;

//--------------------------------------------------------------
// Czas ostatniej istotnej aktywności.
//
// Nie uwzględniamy tutaj zmian samego czasu odtwarzania.
// Dzięki temu zegar może pojawić się również podczas
// ciągłego odtwarzania muzyki.
//--------------------------------------------------------------

uint32_t lastActivityMillis;

};



#endif