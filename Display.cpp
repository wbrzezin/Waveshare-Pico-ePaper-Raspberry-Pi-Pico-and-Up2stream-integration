
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
#include "UTF8Print.h"
#include "Theme.h"
#include "Icons.h"


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
constexpr int TIME_Y           = 100;

constexpr int CURRENT_TIME_X   = 7;
constexpr int TOTAL_TIME_X     = 205;

//--------------------------------------------------------------
// Głośność
//--------------------------------------------------------------

constexpr int VOLUME_Y         = 121;
constexpr int VOLUME_X         = 100;
//--------------------------------------------------------------
// Marginesy ekranu
//--------------------------------------------------------------

constexpr int MARGIN_X         = 10;


//--------------------------------------------------------------
// Parametry obszaru przewijanego tekstu
//--------------------------------------------------------------

constexpr int TEXT_X               = MARGIN_X;
constexpr int TEXT_WIDTH           = 230;


//==============================================================
// Czas wyświetlania jednej strony tekstu [ms]
//==============================================================
constexpr uint32_t PAGE_DISPLAY_TIME = 2000;

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
    epd.setFont(&FONT_STATUS);
    epd.setTextColor(GxEPD_BLACK);

    epd.setFullWindow();

    epd.firstPage();

    do
    {
        epd.fillScreen(GxEPD_WHITE);

        epd.setCursor(10, 40);
        epd.println("MEANDRY TECHNIKI");

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

void Display::initScroll(
    ScrollState& scroll,
    const char* text,
    int areaWidth,
    const GFXfont* font)
{

    //----------------------------------------------------------
    // Zapamiętaj szerokość dostępnego obszaru.
    //----------------------------------------------------------

    scroll.areaWidth = areaWidth;

    //----------------------------------------------------------
    // Obliczenie szerokości całego napisu.
    //
    // Do pomiaru wykorzystywana jest ta sama czcionka,
    // która zostanie użyta podczas rysowania.
    //----------------------------------------------------------

scroll.textWidth =
    measureTextWidth(text, font);

    //----------------------------------------------------------
    // Sprawdź, czy wymagane jest przewijanie.
    //----------------------------------------------------------

    scroll.enabled = (scroll.textWidth > areaWidth);

 //----------------------------------------------------------
 // Wyzeruj stan wyświetlania stron.
 //----------------------------------------------------------

 scroll.currentPage = 0;
 scroll.pageCount   = 0;
 scroll.lastUpdate  = millis();

 //----------------------------------------------------------
 // Wyczyść przygotowane strony.
 //----------------------------------------------------------

 for (uint8_t i = 0; i < 10; i++)
 {
    scroll.pages[i] = "";
 }

 //----------------------------------------------------------
 // Jeżeli tekst mieści się w całości, przygotuj tylko jedną
 // stronę.
 //----------------------------------------------------------

 if (!scroll.enabled)
 {
    scroll.pages[0] = text;
    scroll.pageCount = 1;
    return;
 }

//----------------------------------------------------------
// Podział tekstu na kolejne strony.
//
// Każda strona zawiera maksymalnie dużo pełnych wyrazów,
// które mieszczą się w dostępnym obszarze.
//----------------------------------------------------------

String source(text);

uint16_t start = 0;

while (start < source.length() &&
       scroll.pageCount < 10)
{
    //------------------------------------------------------
    // Pominięcie spacji na początku strony.
    //------------------------------------------------------

    while (start < source.length() &&
           source[start] == ' ')
    {
        start++;
    }

    if (start >= source.length())
        break;

    String bestPage = "";
    uint16_t nextStart = start;

    uint16_t pos = start;

    while (true)
    {
        //--------------------------------------------------
        // Odszukaj koniec kolejnego wyrazu.
        //--------------------------------------------------

        uint16_t end = pos;

        while (end < source.length() &&
               source[end] != ' ')
        {
            end++;
        }

        //--------------------------------------------------
        // Zbuduj kandydata.
        //--------------------------------------------------

        String candidate;

        if (bestPage.length() == 0)
            candidate = source.substring(start, end);
        else
            candidate = bestPage + " " + source.substring(pos, end);

        //--------------------------------------------------
        // Sprawdź szerokość.
        //--------------------------------------------------

        if (measureTextWidth(candidate, font) > areaWidth)
            break;

        bestPage = candidate;
        nextStart = end;

        //--------------------------------------------------
        // Koniec tekstu.
        //--------------------------------------------------

        if (end >= source.length())
            break;

        //--------------------------------------------------
        // Przejście do następnego wyrazu.
        //--------------------------------------------------

        pos = end;

        while (pos < source.length() &&
               source[pos] == ' ')
        {
            pos++;
        }
    }

    //------------------------------------------------------
    // Bardzo długi pojedynczy wyraz.
    //------------------------------------------------------

    if (bestPage.length() == 0)
    {
        uint16_t end = start + 1;

        while (end <= source.length())
        {
            String candidate = source.substring(start, end);

            if (measureTextWidth(candidate, font) > areaWidth)
            {
                end--;
                break;
            }

            end++;
        }

        if (end <= start)
            end = start + 1;

        bestPage = source.substring(start, end);
        nextStart = end;
    }

    //------------------------------------------------------
    // Zapis strony.
    //------------------------------------------------------

    scroll.pages[scroll.pageCount++] = bestPage;

    start = nextStart;
}

 


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

//==============================================================
// Aktualizacja wyświetlania kolejnych stron tekstu
//==============================================================
bool Display::updateScroll(ScrollState& scroll)
{
    //----------------------------------------------------------
    // Jeżeli jest tylko jedna strona, nic nie zmieniamy.
    //----------------------------------------------------------

    if (scroll.pageCount <= 1)
        return false;

    //----------------------------------------------------------
    // Sprawdzenie czasu od ostatniej zmiany strony.
    //----------------------------------------------------------

    uint32_t now = millis();
    

    if ((now - scroll.lastUpdate) < PAGE_DISPLAY_TIME)
        return false;

    //----------------------------------------------------------
    // Zapamiętanie czasu przełączenia.
    //----------------------------------------------------------

    scroll.lastUpdate = now;

    //----------------------------------------------------------
    // Przejście do następnej strony.
    //----------------------------------------------------------

    scroll.currentPage++;

    //----------------------------------------------------------
    // Powrót do pierwszej strony.
    //----------------------------------------------------------

    if (scroll.currentPage >= scroll.pageCount)
        scroll.currentPage = 0;

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
// Częściowe odświeżenie ekranu
//
// Sterownik GDEY0213B74 wykonuje częściowe odświeżenie praktycznie
// z takim samym czasem niezależnie od wielkości okna. Z tego
// powodu obecnie odświeżany jest cały ekran.
//
// Parametr 'changes' pozostaje w funkcji, ponieważ w przyszłości
// może zostać wykorzystany dla innych sterowników lub po dalszej
// optymalizacji kodu.
//==============================================================
void Display::refreshPartial(const PlayerState& player,
                             ChangeFlags changes)
{
    (void)changes;

    //----------------------------------------------------------
    // Ustawienie całego ekranu jako obszaru częściowego
    // odświeżania.
    //----------------------------------------------------------
    epd.setPartialWindow(
        0,
        0,
        epd.width(),
        epd.height());



    //----------------------------------------------------------
    // Narysowanie całego ekranu odtwarzacza.
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

   //----------------------------------------------------------
// Obliczenie wysokości aktualnie ustawionej czcionki.
//
// Każda sekcja interfejsu (tytuł, wykonawca, nagłówek)
// może korzystać z innej czcionki. Z tego powodu wysokość
// czyszczonego obszaru nie powinna być wpisana "na sztywno",
// lecz wyznaczana na podstawie aktualnie ustawionej czcionki.
//----------------------------------------------------------

int16_t x1;
int16_t y1;

uint16_t textWidth;
uint16_t textHeight;

// Pomiar przykładowego znaku.
// Litera "M" posiada zwykle największą wysokość.
epd.getTextBounds(
    "M",
    0,
    0,
    &x1,
    &y1,
    &textWidth,
    &textHeight);

//----------------------------------------------------------
// Wyczyść cały obszar zajmowany przez tekst.
//
// Dodawany jest niewielki margines (2 px), aby usunąć
// ewentualne pozostałości po poprzednim rysowaniu.
//----------------------------------------------------------

epd.fillRect(
    x,
    y - textHeight,
    width,
    textHeight + 2,
    GxEPD_WHITE);

    //----------------------------------------------------------
    // Ustaw pozycję kursora.
    //----------------------------------------------------------

    //----------------------------------------------------------
    // Wyświetlenie tekstu zawsze w stałej pozycji.
    //----------------------------------------------------------
     epd.setCursor(x, y);

   
    //----------------------------------------------------------
    // Narysuj tekst.
    //----------------------------------------------------------
    epd.setTextWrap(false);
    printPL(epd, getVisibleText(text, scroll, width).c_str());
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
    
    //----------------------------------------------------------
    // Inicjalizacja przewijania nazwy wykonawcy.
    //
    // Pomiar wykonywany jest z wykorzystaniem czcionki
    // przeznaczonej dla wykonawcy.
    //----------------------------------------------------------

    initScroll(
        titleScroll,
        player.title,
        TEXT_WIDTH,
        &FONT_TITLE);

              
   }

   //----------------------------------------------------------
   // Jeżeli zmienił się wykonawca,
   // zainicjalizuj przewijanie od początku.
   //----------------------------------------------------------

   if ((changes & ChangeFlags::Artist) != ChangeFlags::None)
   {
    //----------------------------------------------------------
    // Inicjalizacja przewijania nazwy wykonawcy.
    //
    // Pomiar wykonywany jest z wykorzystaniem czcionki
    // przeznaczonej dla wykonawcy.
    //----------------------------------------------------------

    initScroll(
        artistScroll,
        player.artist,
        TEXT_WIDTH,
        &FONT_ARTIST);
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
    // Nagłówek z nazwą źródła + separator (linia oddzielająca).
    //----------------------------------------------------------

        drawHeaderArea(player.source);

    //----------------------------------------------------------
    // Informacje o aktualnie odtwarzanym utworze.
    //----------------------------------------------------------

    drawTrackInfo(player.title,
              player.artist);

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
// Funkcja drawSourceInfo()
//
// Rysuje informacje o aktualnym źródle dźwięku.
//
// Parametry:
//
// source
//      Nazwa aktualnego źródła.
//
// Zwraca:
//
// nic
//
//==============================================================

void Display::drawSourceInfo(const char* source)

{
    //----------------------------------------------------------
    // Wyświetlenie nazwy aktualnego źródła.
    //
    // Zrezygnowano z ikon źródeł (Spotify, Bluetooth, AirPlay,
    // NAS, USB itd.), ponieważ na wyświetlaczu e-paper
    // nie zapewniają wystarczającej czytelności.
    //----------------------------------------------------------

    drawSourceName(source);
}


//==============================================================
// Funkcja drawSourceName()
//
// Rysuje nazwę aktualnego źródła dźwięku.
// Parametry:
// source
//      Nazwa źródła.
// Zwraca:
// nic
//
//==============================================================

void Display::drawSourceName(const char* source)
{
    drawHeader(source);
}

//==============================================================
// Rysowanie nagłówka z nazwą źródła dźwięku.
//==============================================================

void Display::drawHeader(const char* source)
{
    epd.setFont(&FONT_STATUS);
//----------------------------------------------------------
// Nazwa źródła rozpoczyna się od lewego marginesu.
//
// Zrezygnowano z ikon źródeł, dlatego nie jest już
// rezerwowane miejsce na ich wyświetlenie.
//----------------------------------------------------------
    epd.setCursor(
    MARGIN_X,
    HEADER_Y);
    printPL(epd, source);
}

//==============================================================
// Funkcja drawHeaderArea()
//
// Rysuje cały obszar nagłówka.
//
// Parametry:
//
// source
//      Nazwa aktualnego źródła dźwięku.
//
// Zwraca:
//
// nic
//
//==============================================================

void Display::drawHeaderArea(const char* source)
{
    //----------------------------------------------------------
    // Rysowanie nagłówka.
    //----------------------------------------------------------

    drawSourceInfo(source);

    //----------------------------------------------------------
    // Linia oddzielająca nagłówek od pozostałej części ekranu.
    //----------------------------------------------------------

    drawHeaderSeparator();
}

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

void Display::drawTrackInfo(const char* title,
                            const char* artist)
{
    drawTitle(title);

    drawArtist(artist);
}

//==============================================================
// Rysowanie tytułu aktualnie odtwarzanego utworu.
//==============================================================

void Display::drawTitle(const char* title)
{
    epd.setFont(&FONT_TITLE);
    drawScrollingText(title,
                      titleScroll,
                      MARGIN_X,
                      TITLE_Y,
                      230);
}

//--------------------------------------------------------------
// Obliczenie szerokości tekstu.
//
// Funkcja chwilowo ustawia przekazaną czcionkę,
// oblicza szerokość napisu, a następnie zwraca wynik.
//
// Dzięki temu szerokość zawsze odpowiada czcionce,
// która będzie użyta podczas rysowania.
//--------------------------------------------------------------
uint16_t Display::measureTextWidth(
    const String& text,
    const GFXfont* font)
{
    //----------------------------------------------------------
    // Ustawienie czcionki używanej do pomiaru.
    //----------------------------------------------------------

    epd.setFont(font);

    //----------------------------------------------------------
    // Wyznaczenie prostokąta ograniczającego napis.
    //----------------------------------------------------------

    int16_t x1;
    int16_t y1;

    uint16_t w;
    uint16_t h;

    epd.getTextBounds(
        text,
        0,
        0,
        &x1,
        &y1,
        &w,
        &h);

    return w;
}

//==============================================================
// Funkcja scorePage()
//
// Oblicza ocenę jakości podziału tekstu.
//
// Im lepiej wykorzystana jest szerokość strony,
// tym wyższy wynik.
//
// W kolejnych etapach funkcja zostanie rozszerzona o:
// - karę za pozostawienie bardzo krótkiej następnej strony,
// - karę za rozpoczynanie strony od krótkich spójników,
// - premię za bardziej naturalny podział tekstu.
//
// Parametry:
//
// page
//      Kandydat na bieżącą stronę.
//
// remaining
//      Pozostała część tekstu.
//
// areaWidth
//      Maksymalna szerokość strony w pikselach.
//
// Zwraca:
//
// Wynik oceny. Im większa wartość, tym lepszy podział.
//
//==============================================================
int Display::scorePage(const String& page,
                       const String& remaining,
                       int areaWidth)
{
    (void)remaining;

    uint16_t width = measureTextWidth(page, &FONT_TITLE);

    return (100 * width) / areaWidth;
}


//==============================================================
// Rysowanie nazwy wykonawcy.
//==============================================================

void Display::drawArtist(const char* artist)
{
epd.setFont(&FONT_ARTIST);
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
    // Aktualny czas odtwarzania.
    //
    // Ikona stanu odtwarzania jest wyświetlana jako pierwszy
    // znak ciągu tekstowego. Dzięki temu można ocenić wygląd
    // glifu wygenerowanego w czcionce oraz jego wyrównanie
    // względem cyfr czasu.
    //----------------------------------------------------------
epd.setFont(&FONT_TIME);

epd.setCursor(CURRENT_TIME_X, TIME_Y);

printPL(epd, "\x92");
epd.print(" ");
printPL(epd, currentTime);

    //----------------------------------------------------------
    // Parametry paska postępu.
    //----------------------------------------------------------

    const int barX = BAR_X;
    const int barY = BAR_Y;
    const int barWidth = epd.width() - 1;

    //----------------------------------------------------------
// Obliczenie szerokości odtworzonej części paska.
// Parametr progress określa postęp odtwarzania
// w procentach (0...100).
//----------------------------------------------------------

int playedWidth = (barWidth * progress) / 100;

//----------------------------------------------------------
// Obrys całego paska.
//----------------------------------------------------------

epd.drawRect(barX,
             barY - 3,
             barWidth,
             4,
             GxEPD_BLACK);

//----------------------------------------------------------
// Wypełnienie odtworzonej części.
//----------------------------------------------------------

if (playedWidth > 0)
{
    epd.fillRect(barX,
                 barY -2,
                 playedWidth,
                 3,
                 GxEPD_BLACK);
}
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
    epd.setFont(&FONT_VOLUME);

    epd.setCursor(VOLUME_X, VOLUME_Y);

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

void Display::drawHeaderSeparator()
{
    drawSeparator(HEADER_LINE);
}

//==============================================================
// Zwrócenie aktualnie wyświetlanej strony tekstu
//==============================================================
String Display::getVisibleText(
    const String& text,
    ScrollState& scroll,
    int maxWidth)
{
    (void)text;
    (void)maxWidth;

    //----------------------------------------------------------
    // Brak przygotowanych stron.
    //----------------------------------------------------------
    if (scroll.pageCount == 0)
        return "";

    //----------------------------------------------------------
    // Zabezpieczenie przed błędnym indeksem.
    //----------------------------------------------------------
    if (scroll.currentPage >= scroll.pageCount)
        scroll.currentPage = 0;

    //----------------------------------------------------------
    // Zwrócenie aktualnej strony.
    //----------------------------------------------------------
    return scroll.pages[scroll.currentPage];
}