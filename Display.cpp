
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

//--------------------------------------------------------------
// Nagłówek
//--------------------------------------------------------------

constexpr int HEADER_Y         = 19;
constexpr int HEADER_LINE      = 26;

//--------------------------------------------------------------
// Informacje o utworze
//--------------------------------------------------------------

constexpr int TITLE_Y          = 46;
constexpr int ARTIST_Y         = 74;

//--------------------------------------------------------------
// Pasek postępu
//--------------------------------------------------------------

constexpr int BAR_X            = 0;
constexpr int BAR_Y            = 85;

//--------------------------------------------------------------
// Ikona odtwarzania i czasy
//--------------------------------------------------------------

constexpr int PLAY_X           = 10;
constexpr int PLAY_Y           = 98;
constexpr int TIME_Y           = 103;

constexpr int CURRENT_TIME_X   = 22;
constexpr int TOTAL_TIME_X     = 185;

//--------------------------------------------------------------
// Głośność
//--------------------------------------------------------------

constexpr int VOLUME_Y         = 121;

//--------------------------------------------------------------
// Marginesy ekranu
//--------------------------------------------------------------

constexpr int MARGIN_X         = 10;

//--------------------------------------------------------------
// Parametry obszaru przewijanego tekstu
//--------------------------------------------------------------

constexpr int TEXT_X               = MARGIN_X;
constexpr int TEXT_WIDTH           = 230;

//--------------------------------------------------------------
// Parametry animacji przewijania
//--------------------------------------------------------------

constexpr int SCROLL_GAP           = 40;
constexpr int SCROLL_STEP          = 1;
constexpr uint32_t SCROLL_INTERVAL = 40;
constexpr uint32_t SCROLL_PAUSE    = 1500;

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
// Funkcja initScroll()
//
// Oblicza szerokość tekstu oraz określa, czy wymagane jest
// przewijanie.
//
//==============================================================

void Display::initScroll(ScrollState& scroll,
                         const char* text,
                         int areaWidth)
{
    //----------------------------------------------------------
    // Zapamiętaj szerokość dostępnego obszaru.
    //----------------------------------------------------------

    scroll.areaWidth = areaWidth;

    //----------------------------------------------------------
    // Oblicz szerokość napisu.
    //----------------------------------------------------------

    int16_t x1;
    int16_t y1;

    uint16_t w;
    uint16_t h;

    epd.getTextBounds(text,
                      0,
                      0,
                      &x1,
                      &y1,
                      &w,
                      &h);

    scroll.textWidth = w;

    //----------------------------------------------------------
    // Sprawdź, czy wymagane jest przewijanie.
    //----------------------------------------------------------

    scroll.enabled = (w > areaWidth);

    //----------------------------------------------------------
    // Wyzeruj stan animacji.
    //----------------------------------------------------------

    scroll.offset = 0;
    scroll.lastUpdate = millis();

    scroll.pause = false;
    scroll.pauseStart = 0;

//--------------------------------------------------------------
// Informacje diagnostyczne.
//--------------------------------------------------------------

Serial.print("Tekst: ");
Serial.println(text);

Serial.print("Szerokość: ");
Serial.println(scroll.textWidth);

Serial.print("Pole: ");
Serial.println(scroll.areaWidth);

Serial.print("Przewijanie: ");
Serial.println(scroll.enabled ? "TAK" : "NIE");

Serial.println();

}

//==============================================================
// Funkcja updateScroll()
//
// Aktualizuje pozycję przewijanego tekstu.
//
// Funkcja wykonywana jest przy każdym odświeżeniu ekranu.
// Jeżeli przewijanie nie jest wymagane, kończy działanie.
//
//==============================================================

bool Display::updateScroll(ScrollState& scroll)
{
    //----------------------------------------------------------
    // Brak potrzeby przewijania.
    //----------------------------------------------------------

    if (!scroll.enabled)
        return false;

    //----------------------------------------------------------
    // Aktualizacja tylko co określony czas.
    //----------------------------------------------------------

    uint32_t now = millis();

    if (now - scroll.lastUpdate < SCROLL_INTERVAL)
        return false;

    scroll.lastUpdate = now;

    //----------------------------------------------------------
    // Przesunięcie tekstu.
    //----------------------------------------------------------

    scroll.offset += SCROLL_STEP;

    Serial.print("Offset: ");
     Serial.println(scroll.offset);

    //----------------------------------------------------------
    // Tymczasowo po dojściu do końca wracamy na początek.
    // W następnym etapie zastąpimy to pauzami.
    //----------------------------------------------------------

    if (scroll.offset > scroll.textWidth + SCROLL_GAP)
    {
        scroll.offset = 0;
    }
return true;

}

//==============================================================
// Funkcja refreshFull()
//
// Wykonuje pełne odświeżenie wyświetlacza.
//
//==============================================================

void Display::refreshFull(const PlayerState& player)
{
    epd.firstPage();

    do
    {
        drawPlayerScreen(player);
    }
    while (epd.nextPage());
}

//==============================================================
// Funkcja refreshPartial()
//
// Wykonuje częściowe odświeżenie wyświetlacza.
//
// Na obecnym etapie odświeżany jest cały ekran, ale z użyciem
// trybu Partial Update. Pozwala to sprawdzić działanie
// szybkiego odświeżania bez migotania.
//
//==============================================================

void Display::refreshPartial(const PlayerState& player,
                             ChangeFlags changes)
{
    (void)changes;

    //----------------------------------------------------------
    // Ustaw obszar częściowego odświeżania.
    //----------------------------------------------------------

    epd.setPartialWindow(
        0,
        0,
        epd.width(),
        epd.height());

    //----------------------------------------------------------
    // Wykonaj częściowe odświeżenie.
    //----------------------------------------------------------

    epd.firstPage();

    do
    {
        drawPlayerScreen(player);
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

    refreshFull(player);
}


//==============================================================
// Funkcja drawScrollingText()
//
// Rysuje tekst z uwzględnieniem przewijania.
//==============================================================

void Display::drawScrollingText(const char* text,
                                ScrollState& scroll,
                                int x,
                                int y,
                                int width)
{
    //----------------------------------------------------------
    // Wyczyść obszar tekstu.
    //----------------------------------------------------------

    epd.fillRect(x,
                 y - 16,
                 width,
                 20,
                 GxEPD_WHITE);

    //----------------------------------------------------------
    // Ustaw pozycję kursora.
    //----------------------------------------------------------

    if (scroll.enabled)
        epd.setCursor(x - scroll.offset, y);
    else
        epd.setCursor(x, y);

    //----------------------------------------------------------
    // Narysuj tekst.
    //----------------------------------------------------------

    epd.print(text);
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
   // Jeżeli zmienił się tytuł utworu,
   // zainicjalizuj przewijanie od początku.
   //----------------------------------------------------------

   if ((changes & ChangeFlags::Title) != ChangeFlags::None)
   {
    initScroll(titleScroll,
               player.title,
               TEXT_WIDTH);
   }

   //----------------------------------------------------------
   // Jeżeli zmienił się wykonawca,
   // zainicjalizuj przewijanie od początku.
   //----------------------------------------------------------

   if ((changes & ChangeFlags::Artist) != ChangeFlags::None)
   {
    initScroll(artistScroll,
               player.artist,
               TEXT_WIDTH);
   }

   //----------------------------------------------------------
   // Aktualizacja pozycji przewijania.
   //----------------------------------------------------------

    bool titleMoved  = updateScroll(titleScroll);
    bool artistMoved = updateScroll(artistScroll);


 //----------------------------------------------------------
 // Jeżeli wykryto jakąkolwiek zmianę,
 // wykonaj częściowe odświeżenie.
 //
 // Pełne odświeżenie wykonywane jest tylko po uruchomieniu
 // urządzenia.
 //----------------------------------------------------------

  if (changes != ChangeFlags::None ||
    titleMoved ||
    artistMoved)
  {
    refreshPartial(player, changes);
  }
}


//==============================================================
// Funkcja drawPlayerScreen()
//
// Rysuje kompletną zawartość ekranu odtwarzacza.
//
// Funkcja odpowiada wyłącznie za rysowanie elementów
// interfejsu użytkownika.
//
// Nie rozpoczyna ani nie kończy odświeżania wyświetlacza.
//
//==============================================================

void Display::drawPlayerScreen(const PlayerState& player)
{
    //----------------------------------------------------------
    // Nagłówek z nazwą źródła.
    //----------------------------------------------------------

    drawHeader(player.source);

    //----------------------------------------------------------
    // Linia oddzielająca nagłówek.
    //----------------------------------------------------------

    drawSeparator(HEADER_LINE);

    //----------------------------------------------------------
    // Informacje o aktualnie odtwarzanym utworze.
    //----------------------------------------------------------

    drawTitle(player.title);

    drawArtist(player.artist);

    //----------------------------------------------------------
    // Pasek postępu odtwarzania.
    //----------------------------------------------------------

    drawPlaybackBar(player.currentTime,
                    player.totalTime,
                    player.progress);

    //----------------------------------------------------------
    // Aktualny poziom głośności.
    //----------------------------------------------------------

    drawVolume(player.volume);
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
    drawScrollingText(title,
                      titleScroll,
                      MARGIN_X,
                      TITLE_Y,
                      230);
}

//==============================================================
// Rysowanie nazwy wykonawcy.
//==============================================================

void Display::drawArtist(const char* artist)
{
drawScrollingText(artist,
                  artistScroll,
                  MARGIN_X,
                  ARTIST_Y,
                  230);
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