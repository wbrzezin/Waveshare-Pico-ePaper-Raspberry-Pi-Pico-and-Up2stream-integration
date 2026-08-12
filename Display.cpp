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
#include "HardwareConfig.h"
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include "UTF8Print.h"
#include "Theme.h"
#include "Icons.h"
#include "PolishTime.h"
#include "hardware/rtc.h"



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
// Ekran bezczynności
//--------------------------------------------------------------

// Data
constexpr int IDLE_DATE_X      = 160;
constexpr int IDLE_DATE_Y      = 14;

// Zegar
constexpr int IDLE_CLOCK_X     = 78;
constexpr int IDLE_CLOCK_Y     = 66;

//--------------------------------------------------------------
// Wyśrodkowanie zegara.
//
// Szerokość napisu będzie wyznaczana dynamicznie.
//--------------------------------------------------------------

constexpr int IDLE_CLOCK_AREA_X = 0;
constexpr int IDLE_CLOCK_AREA_W = 250;

// Linia oddzielająca
constexpr int IDLE_LINE_Y      = 98;

// Podpis
constexpr int IDLE_FOOTER_X    = 10;
constexpr int IDLE_FOOTER_Y    = 118;


//--------------------------------------------------------------
// Ostatnia minuta wyświetlona na ekranie Idle.
//
// Wartość -1 wymusza pierwszą aktualizację.
//--------------------------------------------------------------
int lastIdleMinute = -1;


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
// Czas bezczynności przed przełączeniem na ekran zegara [ms].
//
// Na etapie testów ustawiamy 5 sekund.
//==============================================================

constexpr uint32_t IDLE_TIMEOUT = 5000;

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


//--------------------------------------------------------------
// Obiekt sterownika wyświetlacza e-paper.
//
// Numery pinów pobierane są z HardwareConfig.h,
// dzięki czemu cała konfiguracja sprzętu znajduje się
// w jednym miejscu.
//--------------------------------------------------------------

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> epd(
    GxEPD2_213_B74(
        DISPLAY_CS_PIN,
        DISPLAY_DC_PIN,
        DISPLAY_RST_PIN,
        DISPLAY_BUSY_PIN));



//==============================================================
// Funkcja setRTC()
//
// Ustawia zegar sprzętowy RTC Raspberry Pi Pico.
//
// Parametry:
//
// time
//      Czas polski obliczony na podstawie komunikatu TME.
//
// Funkcja nie wykonuje żadnych operacji na wyświetlaczu.
// Jej zadaniem jest wyłącznie ustawienie RTC.
//
//==============================================================

void Display::setRTC(
    const PolishTime& time)
{
    Serial.println(
    ">>> Display::setRTC() WYWOŁANA <<<");
    
    //----------------------------------------------------------
    // Jeżeli czas nie jest poprawny, nie zmieniamy RTC.
    //----------------------------------------------------------

    if (!time.valid)
        return;


    //----------------------------------------------------------
    // Przygotowanie struktury wymaganej przez RTC RP2040.
    //----------------------------------------------------------

    datetime_t dateTime =
    {
        .year  = static_cast<int16_t>(time.year),
        .month = static_cast<int8_t>(time.month),
        .day   = static_cast<int8_t>(time.day),

        //------------------------------------------------------
        // RTC RP2040:
        //
        // 0 = niedziela
        // 1 = poniedziałek
        // ...
        // 6 = sobota
        //
        // Na tym etapie nie potrzebujemy jeszcze dokładnego
        // dnia tygodnia do działania zegara, dlatego wartość
        // może pozostać ustawiona na 0.
        //------------------------------------------------------

        .dotw   = 0,

        .hour  = static_cast<int8_t>(time.hour),
        .min   = static_cast<int8_t>(time.minute),
        .sec   = static_cast<int8_t>(time.second)
    };


    //----------------------------------------------------------
    // Ustawienie RTC.
    //----------------------------------------------------------

    rtc_set_datetime(&dateTime);


    //----------------------------------------------------------
    // Diagnostyka synchronizacji.
    //----------------------------------------------------------

    Serial.println(
        "RTC: USTAWIONO CZAS POLSKI");

    Serial.print("RTC DATE = ");

    if (time.day < 10)
        Serial.print('0');

    Serial.print(time.day);

    Serial.print('.');

    if (time.month < 10)
        Serial.print('0');

    Serial.print(time.month);

    Serial.print('.');

    Serial.println(time.year);


    Serial.print("RTC TIME = ");

    if (time.hour < 10)
        Serial.print('0');

    Serial.print(time.hour);

    Serial.print(':');

    if (time.minute < 10)
        Serial.print('0');

    Serial.print(time.minute);

    Serial.print(':');

    if (time.second < 10)
        Serial.print('0');

    Serial.println(time.second);
}


bool Display::begin()
{

//----------------------------------------------------------
// Inicjalizacja zegara RTC w RP2040.
//
// RTC jest uruchamiany podczas inicjalizacji wyświetlacza,
// ale jego czas nie jest tutaj ustawiany.
//
// Aktualny czas zostanie później pobrany z modułu UP2Stream
// poprzez zapytanie TME, przeliczony na czas polski
// i zapisany do RTC.
//----------------------------------------------------------

rtc_init();

//==============================================================
// Konfiguracja magistrali SPI wyświetlacza e-paper.
//
// Wyświetlacz korzysta z dodatkowego interfejsu SPI RP2040
// utworzonego jako SPIn.
//
// Parametry komunikacji:
//
//     częstotliwość = 4 MHz
//     kolejność bitów = MSB first
//     tryb SPI = MODE0
//
//==============================================================

    epd.epd2.selectSPI(
        SPIn,
        SPISettings(4000000, MSBFIRST, SPI_MODE0));

    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);

    epd.init(115200, true, 2, false);

    //----------------------------------------------------------
    // Początkowo wyświetlany jest ekran odtwarzacza.
    //----------------------------------------------------------

    idleScreenActive = false;

    //----------------------------------------------------------
    // Zapamiętujemy moment uruchomienia bezczynności.
    //
    // Dzięki temu ekran zegara pojawi się po IDLE_TIMEOUT
    // milisekundach, jeżeli nie wystąpi żadna istotna zmiana.
    //----------------------------------------------------------

    lastActivityMillis = millis();

    //----------------------------------------------------------
    // Wyświetlenie ekranu startowego.
    //----------------------------------------------------------

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
// Funkcja showIdle()
//
// Wyświetla ekran bezczynności.
//
//==============================================================

void Display::showIdle()
{
    //----------------------------------------------------------
    // Wymuś natychmiastowe narysowanie aktualnej godziny.
    //----------------------------------------------------------

    lastIdleMinute = -1;

    updateIdle();
}

//==============================================================
// Funkcja clearScreen()
//
// Całkowicie czyści ekran e-paper.
//
// Po wykonaniu funkcji wyświetlacz pozostaje biały.
//==============================================================

void Display::clearScreen()
{
    //----------------------------------------------------------
    // Ustawienie całego ekranu jako obszaru odświeżania.
    //----------------------------------------------------------

    epd.setFullWindow();


    //----------------------------------------------------------
    // Rozpoczęcie pełnego odświeżania.
    //----------------------------------------------------------

    epd.firstPage();

    do
    {
        //------------------------------------------------------
        // Wypełnienie całego ekranu kolorem białym.
        //------------------------------------------------------

        epd.fillScreen(GxEPD_WHITE);

    }
    while (epd.nextPage());
}

//==============================================================
// Funkcja updateIdle()
//
// Sprawdza aktualny czas RTC.
//
// Ekran e-paper jest odświeżany tylko wtedy, gdy zmieniła się
// minuta. Dzięki temu nie wykonujemy niepotrzebnych odświeżeń.
//==============================================================

void Display::updateIdle()
{
    datetime_t now;

    //----------------------------------------------------------
    // Pobranie aktualnego czasu z RTC.
    //----------------------------------------------------------

    rtc_get_datetime(&now);

    //----------------------------------------------------------
    // Jeżeli minuta się nie zmieniła, nic nie robimy.
    //----------------------------------------------------------

    if (now.min == lastIdleMinute)
        return;

    //----------------------------------------------------------
    // Zapamiętanie aktualnej minuty.
    //----------------------------------------------------------

    lastIdleMinute = now.min;

    //----------------------------------------------------------
    // Odświeżenie całego ekranu Idle.
    //----------------------------------------------------------

    epd.setPartialWindow(
        0,
        0,
        epd.width(),
        epd.height());

    epd.firstPage();

    do
    {
        drawIdleScreen();
    }
    while (epd.nextPage());
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

    //==========================================================
    // Obsługa trybu standby.
    //
    // SYS:STANDBY powoduje wyczyszczenie całego wyświetlacza.
    //
    // W trybie standby ignorujemy pozostałe zmiany stanu
    // odtwarzacza. Dzięki temu komunikaty takie jak MUT, VOL,
    // ELP itd. nie spowodują ponownego narysowania ekranu.
    //==========================================================

    if (player.standby)
    {
        //------------------------------------------------------
        // Jeżeli właśnie weszliśmy w standby, wyczyść ekran.
        //------------------------------------------------------

        if ((changes & ChangeFlags::Standby)
            != ChangeFlags::None)
        {
            clearScreen();

            //--------------------------------------------------
            // Zapamiętujemy stan ekranu.
            //
            // Ekran nie jest ani odtwarzaczem, ani zegarem.
            //--------------------------------------------------

            idleScreenActive = false;
        }

        //------------------------------------------------------
        // W standby niczego więcej nie rysujemy.
        //------------------------------------------------------

        return;
    }



//==========================================================
// Wyjście z trybu standby.
//
// Po SYS:ON nie rysujemy jeszcze ekranu na podstawie
// starego PlayerState.
//
// Zamiast tego pozwalamy głównej pętli ponownie pobrać
// aktualne źródło z Up2Stream.
//==========================================================

if ((changes & ChangeFlags::Standby)
    != ChangeFlags::None)
{
    if (!player.standby)
    {
        //------------------------------------------------------
        // Urządzenie wróciło do normalnej pracy.
        //------------------------------------------------------

        idleScreenActive = false;

        lastActivityMillis = millis();

        //------------------------------------------------------
        // Nie wyświetlamy jeszcze ekranu.
        //
        // Aktualny stan źródła zostanie pobrany przez:
        //
        //     SRC;
        //
        //------------------------------------------------------

        return;
    }
}

    //----------------------------------------------------------
    // Obsługa przełączania pomiędzy ekranem odtwarzacza
    // a ekranem zegara.
    //
    // Zasada działania:
    //
    // PLAY:
    //     zawsze ekran odtwarzacza.
    //
    // PAUSE / STOP:
    //     ekran odtwarzacza pozostaje przez IDLE_TIMEOUT,
    //     następnie przełączamy się na ekran zegara.
    //
    // PLAY podczas wyświetlania zegara:
    //     natychmiast wracamy do ekranu odtwarzacza.
    //
    // Aktualizacja zegara odbywa się przez updateIdle().
    //----------------------------------------------------------


    //----------------------------------------------------------
    // Sprawdź, czy zmienił się stan PLAY / PAUSE.
    //
    // Jest to podstawowy sygnał sterujący przełączaniem
    // pomiędzy ekranami.
    //----------------------------------------------------------

    bool playStateChanged =
        (changes & ChangeFlags::PlayState)
        != ChangeFlags::None;


    //----------------------------------------------------------
    // Jeżeli zmienił się stan odtwarzania, zapamiętaj moment
    // tej zmiany.
    //
    // Jest to początek odliczania IDLE_TIMEOUT po przejściu
    // z PLAY do PAUSE.
    //----------------------------------------------------------

    if (playStateChanged)
    {
        lastActivityMillis = millis();
    }


    //----------------------------------------------------------
    // PLAY
    //
    // Podczas odtwarzania zawsze pokazujemy ekran odtwarzacza.
    //----------------------------------------------------------

    if (player.playing)
    {
        //------------------------------------------------------
        // Jeżeli zegar był aktualnie wyświetlany, natychmiast
        // wracamy do ekranu odtwarzacza.
        //------------------------------------------------------

        if (idleScreenActive)
        {
            idleScreenActive = false;

            Serial.println(
                "DISPLAY: PLAY -> EKRAN ODTWARZACZA");

            showPlayer(player);

            return;
        }


        //------------------------------------------------------
        // Jeżeli już jesteśmy na ekranie odtwarzacza, niczego
        // nie zmieniamy w związku z mechanizmem Idle.
        //------------------------------------------------------
    }


    //----------------------------------------------------------
    // PAUSE / STOP
    //----------------------------------------------------------

    else
    {
        //------------------------------------------------------
        // Jeżeli zegar jest już aktywny, pozostawiamy go
        // aktywnego, ale aktualizujemy jego zawartość z RTC.
        //------------------------------------------------------

        if (idleScreenActive)
        {
            updateIdle();

            return;
        }


        //------------------------------------------------------
        // Jesteśmy jeszcze na ekranie odtwarzacza.
        //
        // Sprawdź, czy minął czas bezczynności.
        //------------------------------------------------------

        if (millis() - lastActivityMillis >= IDLE_TIMEOUT)
        {
            //--------------------------------------------------
            // Przełączenie na ekran zegara.
            //--------------------------------------------------

            idleScreenActive = true;

            Serial.println(
                "DISPLAY: PAUSE -> EKRAN ZEGARA");

            showIdle();

            return;
        }
    }


    //----------------------------------------------------------
    // Od tego miejsca pozostaje dotychczasowa obsługa ekranu
    // odtwarzacza:
    //
    // - inicjalizacja przewijania tytułu,
    // - inicjalizacja przewijania wykonawcy,
    // - aktualizacja stron tekstu,
    // - częściowe odświeżenie ekranu.
    //----------------------------------------------------------

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
        player.title.c_str(),
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
        player.artist.c_str(),
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
    //----------------------------------------------------------
    // Diagnostyka aktualizacji ekranu.
    //
    // Sprawdzamy, czy każda zmiana głośności dociera
    // do modułu Display.
    //----------------------------------------------------------

    Serial.print("DISPLAY UPDATE  VOL=");
    Serial.print(player.volume);

    Serial.print("  CHANGES=");
    Serial.println(
        static_cast<uint16_t>(changes));

    refreshPartial(player, changes);

    Serial.print("DISPLAY TITLE = [");
    Serial.print(player.title);
    Serial.println("]");
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

//----------------------------------------------------------
// Przygotowanie tekstu źródła.
//
// Dla źródła NET, jeżeli otrzymaliśmy VND, wyświetlamy:
//
//     NET: spotify
//
// Jeżeli VND nie zostało odebrane:
//
//     NET
//
// Dla pozostałych źródeł wyświetlamy wyłącznie SRC.
//----------------------------------------------------------

String sourceText =
    player.source;

if (player.source == "NET" &&
    player.vendor.length() > 0)
{
    sourceText += ": ";
    sourceText += player.vendor;
}


//----------------------------------------------------------
// Nagłówek z nazwą źródła + separator.
//----------------------------------------------------------

drawHeaderArea(
    sourceText.c_str());
    
    //----------------------------------------------------------
    // Informacje o aktualnie odtwarzanym utworze.
    //----------------------------------------------------------

   drawTrackInfo(
    player.title.c_str(),
    player.artist.c_str());

//----------------------------------------------------------
// Pasek postępu odtwarzania.
//
// Przekazujemy również aktualny stan odtwarzania,
// aby ikona mogła pokazać:
//
// PLAY  - podczas odtwarzania,
// PAUSE - podczas pauzy.
//----------------------------------------------------------

   drawPlaybackBar(
    player.currentTime,
    player.totalTime,
    player.progress,
    player.playing);

    //----------------------------------------------------------
    // Aktualny poziom głośności.
    //----------------------------------------------------------

    drawVolume(player.volume);
}

//==============================================================
// Funkcja drawIdleScreen()
//
// Rysuje ekran bezczynności.
//
// Na obecnym etapie wyświetlane są przykładowa data,
// przykładowa godzina oraz podpis urządzenia.
//==============================================================

void Display::drawIdleScreen()
{
    //----------------------------------------------------------
    // Wyczyść cały ekran.
    //----------------------------------------------------------

    epd.fillScreen(GxEPD_WHITE);

        //----------------------------------------------------------
    // Pobranie aktualnej daty i godziny z RTC.
    //----------------------------------------------------------

    datetime_t now;

    rtc_get_datetime(&now);

    //----------------------------------------------------------
    // Data.
    //
    // Format:
    //
    // DD.MM.RRRR
    //----------------------------------------------------------

    epd.setFont(&FONT_STATUS);

    epd.setCursor(
        IDLE_DATE_X,
        IDLE_DATE_Y);

    char dateText[11];

    snprintf(
        dateText,
        sizeof(dateText),
        "%02d.%02d.%04d",
        now.day,
        now.month,
        now.year);

    epd.print(dateText);

    //----------------------------------------------------------
    // Godzina.
    //----------------------------------------------------------

//----------------------------------------------------------
// Wyświetlenie dużego zegara.
//
// Pozycja pozioma wyznaczana jest automatycznie,
// dzięki czemu każda godzina będzie idealnie
// wyśrodkowana.
//----------------------------------------------------------

epd.setFont(&FONT_CLOCK);

char clockBuffer[6];

snprintf(
    clockBuffer,
    sizeof(clockBuffer),
    "%02d:%02d",
    now.hour,
    now.min);

String clockText = clockBuffer;

epd.setCursor(
    calculateCenteredX(
        clockText,
        &FONT_CLOCK,
        IDLE_CLOCK_AREA_X,
        IDLE_CLOCK_AREA_W),
    IDLE_CLOCK_Y);

epd.print(clockText);   

    //----------------------------------------------------------
    // Linia oddzielająca podpis.
    //----------------------------------------------------------

    epd.drawLine(
    0,
    IDLE_LINE_Y,
    epd.width() - 1,
    IDLE_LINE_Y,
    GxEPD_BLACK);

    //----------------------------------------------------------
    // Podpis urządzenia.
    //----------------------------------------------------------

    epd.setFont(&FONT_STATUS);

    epd.setCursor(
    IDLE_FOOTER_X,
    IDLE_FOOTER_Y);

    epd.print("MEANDRY TECHNIKI");
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
    Serial.print("DRAW TITLE = [");
    Serial.print(title);
    Serial.print("]  pages=");
    Serial.print(titleScroll.pageCount);
    Serial.print("  page=");
    Serial.println(titleScroll.currentPage);

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
// Funkcja calculateCenteredX()
//
// Oblicza współrzędną X umożliwiającą wyśrodkowanie napisu
// w zadanym obszarze.
//==============================================================

int Display::calculateCenteredX(
    const String& text,
    const GFXfont* font,
    int areaX,
    int areaWidth)
{
    //----------------------------------------------------------
    // Obliczenie szerokości napisu.
    //----------------------------------------------------------

    uint16_t width =
        measureTextWidth(text, font);

    //----------------------------------------------------------
    // Wyznaczenie współrzędnej X.
    //----------------------------------------------------------

    return areaX + (areaWidth - width) / 2;
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

void Display::drawPlaybackBar(
    const char* currentTime,
    const char* totalTime,
    int progress,
    bool playing)
{

    //----------------------------------------------------------
    // Diagnostyka danych przekazywanych do paska odtwarzania.
    //----------------------------------------------------------

    Serial.print("DRAW PLAYBACK  ");

    Serial.print(currentTime);

    Serial.print(" / ");

    Serial.print(totalTime);

    Serial.print("  PROGRESS=");

    Serial.println(progress);


//----------------------------------------------------------
// Ikona stanu odtwarzania.
//
// PLAY  - gdy odtwarzanie jest aktywne.
// PAUSE - gdy odtwarzanie jest wstrzymane.
//----------------------------------------------------------

epd.setFont(&FONT_TIME);

epd.setCursor(CURRENT_TIME_X, TIME_Y);

if (playing)
{
    //------------------------------------------------------
    // Ikona PLAY - trójkąt.
    //------------------------------------------------------

    printPL(epd, "\x92");
}
else
{
    //------------------------------------------------------
    // Ikona PAUSE - dwie pionowe kreski.
    //------------------------------------------------------

    printPL(epd, "\x93");
}

epd.print("  ");
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