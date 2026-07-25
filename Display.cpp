
//==============================================================
// Projekt : UP2Stream Display
// Plik    : Display.cpp
// Autor   : Waldemar Brzeziński
// Opis    : Implementacja klasy odpowiedzialnej za obsługę
//           wyświetlacza e-paper.
//==============================================================

//==============================================================
// Dołączone biblioteki
//
// Biblioteki wymagane do obsługi wyświetlacza e-paper,
// magistrali SPI oraz wykorzystywanej czcionki.
//==============================================================

#include "Display.h"
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>


//==============================================================
// Definicje współrzędnych elementów interfejsu
//
// Wszystkie współrzędne zostały zebrane w jednym miejscu.
// Dzięki temu późniejsza zmiana wyglądu ekranu wymaga jedynie
// modyfikacji poniższych stałych.
//==============================================================

constexpr int HEADER_Y         = 19;
constexpr int HEADER_LINE      = 26;

constexpr int TITLE_Y          = 46;
constexpr int ARTIST_Y         = 74;

constexpr int BAR_X            = 0;
constexpr int BAR_Y            = 85;

constexpr int PLAY_X           = 10;
constexpr int PLAY_Y           = 98;
constexpr int TIME_Y           = 103;

constexpr int VOLUME_Y         = 121;

constexpr int MARGIN_X         = 10;

constexpr int CURRENT_TIME_X   = 22;
constexpr int TOTAL_TIME_X     = 185;



//==============================================================
// Konfiguracja interfejsu SPI dla Raspberry Pi Pico RP2040
//
// Wyświetlacz korzysta z drugiego kontrolera SPI (SPI1),
// dzięki czemu podstawowy interfejs SPI pozostaje dostępny
// dla innych urządzeń.
//==============================================================

#if defined(ARDUINO_ARCH_RP2040)
SPIClassRP2040 SPIn(spi1, 12, 13, 10, 11);
#endif


//==============================================================
// Utworzenie obiektu wyświetlacza
//
// Parametry konstruktora:
// CS   - GP9
// DC   - GP8
// RST  - GP12
// BUSY - GP13
//==============================================================

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> epd(
    GxEPD2_213_B74(9, 8, 12, 13));


//==============================================================
// Inicjalizacja wyświetlacza
//
// Funkcja konfiguruje magistralę SPI, uruchamia sterownik
// wyświetlacza oraz wyświetla ekran startowy.
//==============================================================

bool Display::begin()
{
    epd.epd2.selectSPI(SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));

    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);

    epd.init(115200, true, 2, false);

    splash();

    return true;
}


//==============================================================
// Wyświetlenie ekranu startowego
//
// Funkcja wykonywana jednorazowo podczas uruchamiania
// urządzenia.
//==============================================================

void Display::splash()
{
    epd.setRotation(1);
    epd.setFont(&FreeMonoBold9pt7b);
    epd.setTextColor(GxEPD_BLACK);

    epd.setFullWindow();

    epd.firstPage();

    do
    {
        epd.fillScreen(GxEPD_WHITE);

        epd.setCursor(10, 40);
        epd.println("UP2STREAM");

        epd.setCursor(10, 70);
        epd.println("Display OK");
    }
    while (epd.nextPage());
}


//==============================================================
// Funkcja showPlayer()
//
// Wyświetla główny ekran odtwarzacza.
//
// Funkcja rysuje wszystkie elementy interfejsu użytkownika
// na podstawie informacji zawartych w strukturze PlayerState.
//
// Parametry:
//
// player
//      Aktualny stan odtwarzacza.
//
//==============================================================

void Display::showPlayer(const PlayerState& player)
{
    Serial.println("showPlayer()");

    //----------------------------------------------------------
    // Rozpoczęcie pełnego odświeżania wyświetlacza.
    //----------------------------------------------------------

    epd.firstPage();

    do
    {
        //------------------------------------------------------
        // Nagłówek z nazwą źródła dźwięku.
        //------------------------------------------------------

        drawHeader(player.source);

        //------------------------------------------------------
        // Linia oddzielająca nagłówek od pozostałej części
        // ekranu.
        //------------------------------------------------------

        drawSeparator(HEADER_LINE);

        //------------------------------------------------------
        // Informacje o aktualnie odtwarzanym utworze.
        //------------------------------------------------------

        drawTitle(player.title);

        drawArtist(player.artist);

        //------------------------------------------------------
        // Pasek postępu odtwarzania.
        //------------------------------------------------------

        drawPlaybackBar(player.currentTime,
                        player.totalTime,
                        player.progress);

        //------------------------------------------------------
        // Aktualny poziom głośności.
        //------------------------------------------------------

        drawVolume(player.volume);

    }
    while (epd.nextPage());

 
}

//==============================================================
// Funkcja update()
//
// Aktualizuje zawartość wyświetlacza.
//
// Parametry:
//
// player
//      Aktualny stan odtwarzacza.
//
// changes
//      Zestaw flag określających wykryte zmiany.
//
//==============================================================

void Display::update(const PlayerState& player,
                     ChangeFlags changes)
{
    //----------------------------------------------------------
    // Parametr będzie wykorzystywany w kolejnych etapach
    // projektu.
    //----------------------------------------------------------

    (void)changes;

    //----------------------------------------------------------
    // Tymczasowo wykonywane jest pełne odświeżenie ekranu.
    //----------------------------------------------------------

    showPlayer(player);
}


//==============================================================
// Rysowanie nagłówka z nazwą źródła dźwięku.
//==============================================================

void Display::drawHeader(const char* source)
{
    epd.setCursor(MARGIN_X, HEADER_Y);
    epd.print(source);
}


//==============================================================
// Rysowanie tytułu aktualnie odtwarzanego utworu.
//==============================================================

void Display::drawTitle(const char* title)
{
    epd.setCursor(MARGIN_X, TITLE_Y);
    epd.print(title);
}


//==============================================================
// Rysowanie nazwy wykonawcy.
//==============================================================

void Display::drawArtist(const char* artist)
{
    epd.setCursor(MARGIN_X, ARTIST_Y);
    epd.print(artist);
}


//==============================================================
// Rysowanie paska postępu odtwarzania
//
// Funkcja rysuje:
// - ikonę Play,
// - aktualny czas,
// - pasek postępu,
// - znacznik aktualnej pozycji,
// - całkowity czas utworu.
//==============================================================

void Display::drawPlaybackBar(const char* currentTime,
                              const char* totalTime,
                              int progress)
{
    //----------------------------------------------------------
    // Ikona Play.
    //----------------------------------------------------------

    const int playX = PLAY_X;
    const int playY = PLAY_Y;

    epd.fillTriangle(
        playX,     playY - 4,
        playX,     playY + 4,
        playX + 8, playY,
        GxEPD_BLACK);

    //----------------------------------------------------------
    // Aktualny czas odtwarzania.
    //----------------------------------------------------------

    epd.setCursor(CURRENT_TIME_X, TIME_Y);
    epd.print(currentTime);

    //----------------------------------------------------------
    // Parametry paska postępu.
    //----------------------------------------------------------

    const int barX = BAR_X;
    const int barY = BAR_Y;
    const int barWidth = epd.width() - 1;

    //----------------------------------------------------------
    // Rysowanie linii paska.
    //----------------------------------------------------------

    epd.drawLine(barX,
                 barY,
                 barX + barWidth,
                 barY,
                 GxEPD_BLACK);

    //----------------------------------------------------------
    // Obliczenie położenia znacznika.
    // Parametr progress określa postęp odtwarzania
    // w procentach (0...100).
    //----------------------------------------------------------

    int markerX = barX + (barWidth * progress) / 100;

    //----------------------------------------------------------
    // Rysowanie znacznika aktualnej pozycji.
    //----------------------------------------------------------

    epd.fillCircle(markerX,
                   barY,
                   3,
                   GxEPD_BLACK);

    //----------------------------------------------------------
    // Całkowity czas utworu.
    //----------------------------------------------------------

    epd.setCursor(TOTAL_TIME_X, TIME_Y);
    epd.print(totalTime);
}

//==============================================================
// Funkcja drawVolume()
//
// Rysuje aktualny poziom głośności.
//
// Parametry:
//
// volume
//      Aktualny poziom głośności (0...100%).
//
//==============================================================

void Display::drawVolume(int volume)
{
    //----------------------------------------------------------
    // Ustawienie pozycji kursora.
    //----------------------------------------------------------

    epd.setCursor(0, VOLUME_Y);

    //----------------------------------------------------------
    // Wyświetlenie poziomu głośności.
    //----------------------------------------------------------

    epd.print("VOL ");

    epd.print(volume);

    epd.print("%");
}

//==============================================================
// Rysowanie poziomej linii oddzielającej sekcje ekranu.
//==============================================================

void Display::drawSeparator(int y)
{
    epd.drawLine(0,
                 y,
                 epd.width() - 1,
                 y,
                 GxEPD_BLACK);
}