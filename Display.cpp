//==============================================================
// Projekt : UP2Stream Display                              // Project: UP2Stream Display                    //
// Plik    : Display.cpp                                    // File: Display.cpp                             //
// Autor   : Waldemar Brzeziński                            // Author: Waldemar Brzeziński                   //
// Opis    : Implementacja klasy odpowiedzialnej za obsługę // Description: Implementation of the class responsible for handling //
// wyświetlacza e-paper.                                    // the e-paper display.                          //
//==============================================================

//==============================================================
// Dołączone biblioteki                                     // Included libraries                            //
//
// Biblioteki wymagane do obsługi wyświetlacza e-paper,     // Libraries required for the e-paper display,   //
// magistrali SPI oraz wykorzystywanej czcionki.            // the SPI bus and the selected font.            //
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
// Definicje współrzędnych elementów interfejsu             // Definitions of interface element coordinates  //
//
// Wszystkie współrzędne zostały zebrane w jednym miejscu.  // All coordinates are collected in one place.   //
// Dzięki temu późniejsza zmiana wyglądu ekranu wymaga jedynie // This makes later changes to the screen layout require only //
// modyfikacji poniższych stałych.                          // modifying the constants below.                //
//==============================================================

//--------------------------------------------------------------
// Nagłówek                                                 // Header                                        //
//--------------------------------------------------------------

constexpr int HEADER_Y         = 19;
constexpr int HEADER_LINE      = 26;

//--------------------------------------------------------------
// Informacje o utworze                                     // Track information                             //
//--------------------------------------------------------------

constexpr int TITLE_Y          = 46;
constexpr int ARTIST_Y         = 74;

//--------------------------------------------------------------
// Pasek postępu                                            // Progress bar                                  //
//--------------------------------------------------------------

constexpr int BAR_X            = 0;
constexpr int BAR_Y            = 85;

//--------------------------------------------------------------
// Ikona odtwarzania i czasy                                // Playback icon and times                       //
//--------------------------------------------------------------

constexpr int PLAY_X           = 10;
constexpr int PLAY_Y           = 98;
constexpr int TIME_Y           = 100;

constexpr int CURRENT_TIME_X   = 7;
constexpr int TOTAL_TIME_X     = 205;

//--------------------------------------------------------------
// Głośność                                                 // Volume                                        //
//--------------------------------------------------------------

constexpr int VOLUME_Y         = 121;
constexpr int VOLUME_X         = 100;

//--------------------------------------------------------------
// Ekran bezczynności                                       // Idle screen                                   //
//--------------------------------------------------------------

// Data                                                     // Date                                          //
constexpr int IDLE_DATE_X      = 160;
constexpr int IDLE_DATE_Y      = 14;

// Zegar                                                    // Clock                                         //
constexpr int IDLE_CLOCK_X     = 78;
constexpr int IDLE_CLOCK_Y     = 66;

//--------------------------------------------------------------
// Wyśrodkowanie zegara.                                    // Clock centering.                              //
//
// Szerokość napisu będzie wyznaczana dynamicznie.          // The text width will be determined dynamically. //
//--------------------------------------------------------------

constexpr int IDLE_CLOCK_AREA_X = 0;
constexpr int IDLE_CLOCK_AREA_W = 250;

// Linia oddzielająca                                       // Separator line                                //
constexpr int IDLE_LINE_Y      = 98;

// Podpis                                                   // Footer text                                   //
constexpr int IDLE_FOOTER_X    = 10;
constexpr int IDLE_FOOTER_Y    = 118;


//--------------------------------------------------------------
// Ostatnia minuta wyświetlona na ekranie Idle.             // Last minute displayed on the Idle screen.     //
//
// Wartość -1 wymusza pierwszą aktualizację.                // The value -1 forces the first update.         //
//--------------------------------------------------------------
int lastIdleMinute = -1;


//--------------------------------------------------------------
// Marginesy ekranu                                         // Screen margins                                //
//--------------------------------------------------------------

constexpr int MARGIN_X         = 10;


//--------------------------------------------------------------
// Parametry obszaru przewijanego tekstu                    // Text page area parameters                     //
//--------------------------------------------------------------

constexpr int TEXT_X               = MARGIN_X;
constexpr int TEXT_WIDTH           = 230;


//==============================================================
// Czas wyświetlania jednej strony tekstu [ms]              // Display time for one text page [ms]           //
//==============================================================
constexpr uint32_t PAGE_DISPLAY_TIME = 2000;

//==============================================================
// Czas bezczynności przed przełączeniem na ekran zegara [ms]. // Idle time before switching to the clock screen [ms]. //
//
// Na etapie testów ustawiamy 5 sekund.                     // Set to 5 seconds during testing.              //
//==============================================================

constexpr uint32_t IDLE_TIMEOUT = 5000;

//==============================================================
// Konfiguracja interfejsu SPI dla Raspberry Pi Pico RP2040 // SPI interface configuration for the Raspberry Pi Pico RP2040 //
//
// Wyświetlacz korzysta z drugiego kontrolera SPI (SPI1),   // The display uses the second SPI controller (SPI1), //
// dzięki czemu podstawowy interfejs SPI pozostaje dostępny // leaving the primary SPI interface available   //
// dla innych urządzeń.                                     // for other devices.                            //
//==============================================================

#if defined(ARDUINO_ARCH_RP2040)
SPIClassRP2040 SPIn(spi1, 12, 13, 10, 11);
#endif


//--------------------------------------------------------------
// Obiekt sterownika wyświetlacza e-paper.                  // E-paper display driver object.                //
//
// Numery pinów pobierane są z HardwareConfig.h,            // Pin numbers are read from HardwareConfig.h,   //
// dzięki czemu cała konfiguracja sprzętu znajduje się      // so the entire hardware configuration is kept  //
// w jednym miejscu.                                        // in one place.                                 //
//--------------------------------------------------------------

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> epd(
    GxEPD2_213_B74(
        DISPLAY_CS_PIN,
        DISPLAY_DC_PIN,
        DISPLAY_RST_PIN,
        DISPLAY_BUSY_PIN));



//==============================================================
// Funkcja setRTC()                                         // Function setRTC()                             //
//
// Ustawia zegar sprzętowy RTC Raspberry Pi Pico.           // Sets the Raspberry Pi Pico hardware RTC.      //
//
// Parametry:                                               // Parameters:                                   //
//
// time                                                     // time                                          //
// Czas polski obliczony na podstawie komunikatu TME.       // Polish local time calculated from the TME message. //
//
// Funkcja nie wykonuje żadnych operacji na wyświetlaczu.   // The function performs no operations on the display. //
// Jej zadaniem jest wyłącznie ustawienie RTC.              // Its only task is to set the RTC.              //
//
//==============================================================

void Display::setRTC(
    const PolishTime& time)
{
    Serial.println(
    ">>> Display::setRTC() WYWOŁANA <<<");
    
    //----------------------------------------------------------
    // Jeżeli czas nie jest poprawny, nie zmieniamy RTC.        // If the time is invalid, the RTC is not changed. //
    //----------------------------------------------------------

    if (!time.valid)
        return;


    //----------------------------------------------------------
    // Przygotowanie struktury wymaganej przez RTC RP2040.      // Preparation of the structure required by the RP2040 RTC. //
    //----------------------------------------------------------

    datetime_t dateTime =
    {
        .year  = static_cast<int16_t>(time.year),
        .month = static_cast<int8_t>(time.month),
        .day   = static_cast<int8_t>(time.day),

        //------------------------------------------------------
        // RTC RP2040:                                              // RP2040 RTC:                                   //
        //
        // 0 = niedziela                                            // 0 = Sunday                                    //
        // 1 = poniedziałek                                         // 1 = Monday                                    //
        // ...                                                      // ...                                           //
        // 6 = sobota                                               // 6 = Saturday                                  //
        //
        // Na tym etapie nie potrzebujemy jeszcze dokładnego        // At this stage, an exact                       //
        // dnia tygodnia do działania zegara, dlatego wartość       // day of the week is not required for clock operation, so the value //
        // może pozostać ustawiona na 0.                            // can remain set to 0.                          //
        //------------------------------------------------------

        .dotw   = 0,

        .hour  = static_cast<int8_t>(time.hour),
        .min   = static_cast<int8_t>(time.minute),
        .sec   = static_cast<int8_t>(time.second)
    };


    //----------------------------------------------------------
    // Ustawienie RTC.                                          // Set the RTC.                                  //
    //----------------------------------------------------------

    rtc_set_datetime(&dateTime);


    //----------------------------------------------------------
    // Diagnostyka synchronizacji.                              // Synchronization diagnostics.                  //
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
// Inicjalizacja zegara RTC w RP2040.                       // Initialize the RTC on the RP2040.             //
//
// RTC jest uruchamiany podczas inicjalizacji wyświetlacza, // The RTC is started during display initialization, //
// ale jego czas nie jest tutaj ustawiany.                  // but its time is not set here.                 //
//
// Aktualny czas zostanie później pobrany z modułu UP2Stream // The current time will later be obtained from the UP2Stream module //
// poprzez zapytanie TME, przeliczony na czas polski        // using the TME query, converted to Polish local time //
// i zapisany do RTC.                                       // and stored in the RTC.                        //
//----------------------------------------------------------

rtc_init();

//==============================================================
// Konfiguracja magistrali SPI wyświetlacza e-paper.        // Configuration of the e-paper display SPI bus. //
//
// Wyświetlacz korzysta z dodatkowego interfejsu SPI RP2040 // The display uses the additional RP2040 SPI interface //
// utworzonego jako SPIn.                                   // created as SPIn.                              //
//
// Parametry komunikacji:                                   // Communication parameters:                     //
//
// częstotliwość = 4 MHz                                    // frequency = 4 MHz                             //
// kolejność bitów = MSB first                              // bit order = MSB first                         //
// tryb SPI = MODE0                                         // SPI mode = MODE0                              //
//
//==============================================================

    epd.epd2.selectSPI(
        SPIn,
        SPISettings(4000000, MSBFIRST, SPI_MODE0));

    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);

    epd.init(115200, true, 2, false);

    //----------------------------------------------------------
    // Początkowo wyświetlany jest ekran odtwarzacza.           // Initially display the player screen.          //
    //----------------------------------------------------------

    idleScreenActive = false;

    //----------------------------------------------------------
    // Zapamiętujemy moment uruchomienia bezczynności.          // Remember the time when idle mode started.     //
    //
    // Dzięki temu ekran zegara pojawi się po IDLE_TIMEOUT      // This allows the clock screen to appear after IDLE_TIMEOUT //
    // milisekundach, jeżeli nie wystąpi żadna istotna zmiana.  // milliseconds if no significant change occurs. //
    //----------------------------------------------------------

    lastActivityMillis = millis();

    //----------------------------------------------------------
    // Wyświetlenie ekranu startowego.                          // Display the startup screen.                   //
    //----------------------------------------------------------

    splash();

    return true;
}


//==============================================================
// Wyświetlenie ekranu startowego                           // Display the startup screen                    //
//
// Funkcja wykonywana jednorazowo podczas uruchamiania      // Function executed once during device startup. //
// urządzenia.                                              // of the device.                                //
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
// Funkcja initScroll()                                     // Function initScroll()                         //
//
// Oblicza szerokość tekstu oraz określa, czy wymagane jest // Calculates the text width and determines whether //
// przewijanie.                                             // page switching is required.                   //
//
//==============================================================

void Display::initScroll(
    ScrollState& scroll,
    const char* text,
    int areaWidth,
    const GFXfont* font)
{

    //----------------------------------------------------------
    // Zapamiętaj szerokość dostępnego obszaru.                 // Remember the available area width.            //
    //----------------------------------------------------------

    scroll.areaWidth = areaWidth;

    //----------------------------------------------------------
    // Obliczenie szerokości całego napisu.                     // Calculate the total text width.               //
    //
    // Do pomiaru wykorzystywana jest ta sama czcionka,         // The same font is used for measurement         //
    // która zostanie użyta podczas rysowania.                  // as will be used for drawing.                  //
    //----------------------------------------------------------

scroll.textWidth =
    measureTextWidth(text, font);

    //----------------------------------------------------------
    // Sprawdź, czy wymagane jest przewijanie.                  // Check whether page switching is required.     //
    //----------------------------------------------------------

    scroll.enabled = (scroll.textWidth > areaWidth);

 //----------------------------------------------------------
 // Wyzeruj stan wyświetlania stron.                         // Reset the page display state.                 //
 //----------------------------------------------------------

 scroll.currentPage = 0;
 scroll.pageCount   = 0;
 scroll.lastUpdate  = millis();

 //----------------------------------------------------------
 // Wyczyść przygotowane strony.                             // Clear the prepared pages.                     //
 //----------------------------------------------------------

 for (uint8_t i = 0; i < 10; i++)
 {
    scroll.pages[i] = "";
 }

 //----------------------------------------------------------
 // Jeżeli tekst mieści się w całości, przygotuj tylko jedną // If the text fits completely, prepare only one //
 // stronę.                                                  // page.                                         //
 //----------------------------------------------------------

 if (!scroll.enabled)
 {
    scroll.pages[0] = text;
    scroll.pageCount = 1;
    return;
 }

//----------------------------------------------------------
// Podział tekstu na kolejne strony.                        // Split the text into successive pages.         //
//
// Każda strona zawiera maksymalnie dużo pełnych wyrazów,   // Each page contains as many complete words as possible //
// które mieszczą się w dostępnym obszarze.                 // that fit within the available area.           //
//----------------------------------------------------------

String source(text);

uint16_t start = 0;

while (start < source.length() &&
       scroll.pageCount < 10)
{
    //------------------------------------------------------
    // Pominięcie spacji na początku strony.                    // Skip spaces at the beginning of the page.     //
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
        // Odszukaj koniec kolejnego wyrazu.                        // Find the end of the next word.                //
        //--------------------------------------------------

        uint16_t end = pos;

        while (end < source.length() &&
               source[end] != ' ')
        {
            end++;
        }

        //--------------------------------------------------
        // Zbuduj kandydata.                                        // Build a candidate page.                       //
        //--------------------------------------------------

        String candidate;

        if (bestPage.length() == 0)
            candidate = source.substring(start, end);
        else
            candidate = bestPage + " " + source.substring(pos, end);

        //--------------------------------------------------
        // Sprawdź szerokość.                                       // Check the width.                              //
        //--------------------------------------------------

        if (measureTextWidth(candidate, font) > areaWidth)
            break;

        bestPage = candidate;
        nextStart = end;

        //--------------------------------------------------
        // Koniec tekstu.                                           // End of text.                                  //
        //--------------------------------------------------

        if (end >= source.length())
            break;

        //--------------------------------------------------
        // Przejście do następnego wyrazu.                          // Move to the next word.                        //
        //--------------------------------------------------

        pos = end;

        while (pos < source.length() &&
               source[pos] == ' ')
        {
            pos++;
        }
    }

    //------------------------------------------------------
    // Bardzo długi pojedynczy wyraz.                           // Very long single word.                        //
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
    // Zapis strony.                                            // Store the page.                               //
    //------------------------------------------------------

    scroll.pages[scroll.pageCount++] = bestPage;

    start = nextStart;
}

 


}

//==============================================================
// Funkcja updateScroll()                                   // Function updateScroll()                       //
//
// Aktualizuje pozycję przewijanego tekstu.                 // Updates the currently displayed text page.    //
//
// Funkcja wykonywana jest przy każdym odświeżeniu ekranu.  // The function is called on every display update. //
// Jeżeli przewijanie nie jest wymagane, kończy działanie.  // If page switching is not required, it returns. //
//
//==============================================================

//==============================================================
// Aktualizacja wyświetlania kolejnych stron tekstu         // Update the display of successive text pages   //
//==============================================================
bool Display::updateScroll(ScrollState& scroll)
{
    //----------------------------------------------------------
    // Jeżeli jest tylko jedna strona, nic nie zmieniamy.       // If there is only one page, nothing is changed. //
    //----------------------------------------------------------

    if (scroll.pageCount <= 1)
        return false;

    //----------------------------------------------------------
    // Sprawdzenie czasu od ostatniej zmiany strony.            // Check the time since the last page change.    //
    //----------------------------------------------------------

    uint32_t now = millis();
    

    if ((now - scroll.lastUpdate) < PAGE_DISPLAY_TIME)
        return false;

    //----------------------------------------------------------
    // Zapamiętanie czasu przełączenia.                         // Remember the page switch time.                //
    //----------------------------------------------------------

    scroll.lastUpdate = now;

    //----------------------------------------------------------
    // Przejście do następnej strony.                           // Move to the next page.                        //
    //----------------------------------------------------------

    scroll.currentPage++;

    //----------------------------------------------------------
    // Powrót do pierwszej strony.                              // Return to the first page.                     //
    //----------------------------------------------------------

    if (scroll.currentPage >= scroll.pageCount)
        scroll.currentPage = 0;

    return true;


    
}

//==============================================================
// Funkcja refreshFull()                                    // Function refreshFull()                        //
//
// Wykonuje pełne odświeżenie wyświetlacza.                 // Performs a full display refresh.              //
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
// Częściowe odświeżenie ekranu                             // Partial display refresh                       //
//
// Sterownik GDEY0213B74 wykonuje częściowe odświeżenie praktycznie // The GDEY0213B74 driver performs partial refresh in practically //
// z takim samym czasem niezależnie od wielkości okna. Z tego // the same amount of time regardless of window size. //
// powodu obecnie odświeżany jest cały ekran.               // For this reason, the entire screen is currently refreshed. //
//
// Parametr 'changes' pozostaje w funkcji, ponieważ w przyszłości // The 'changes' parameter remains in the function because it may //
// może zostać wykorzystany dla innych sterowników lub po dalszej // be used in the future with other drivers or after further //
// optymalizacji kodu.                                      // code optimization.                            //
//==============================================================
void Display::refreshPartial(const PlayerState& player,
                             ChangeFlags changes)
{
    (void)changes;

    //----------------------------------------------------------
    // Ustawienie całego ekranu jako obszaru częściowego        // Set the entire screen as the partial refresh area. //
    // odświeżania.                                             // refreshing.                                   //
    //----------------------------------------------------------
    epd.setPartialWindow(
        0,
        0,
        epd.width(),
        epd.height());



    //----------------------------------------------------------
    // Narysowanie całego ekranu odtwarzacza.                   // Draw the entire player screen.                //
    //----------------------------------------------------------
    epd.firstPage();

    do
{
     drawPlayerScreen(player);
}
while (epd.nextPage());

}

//==============================================================
// Funkcja showPlayer()                                     // Function showPlayer()                         //
//
// Wyświetla główny ekran odtwarzacza.                      // Displays the main player screen.              //
//
// Funkcja rysuje wszystkie elementy interfejsu użytkownika // The function draws all user interface elements //
// na podstawie informacji zawartych w strukturze PlayerState. // based on the information contained in the PlayerState structure. //
//
// Parametry:                                               // Parameters:                                   //
//
// player                                                   // player                                        //
// Aktualny stan odtwarzacza.                               // Current player state.                         //
//
//==============================================================

void Display::showPlayer(const PlayerState& player)
{
    Serial.println("showPlayer()");

    refreshFull(player);
}

//==============================================================
// Funkcja showIdle()                                       // Function showIdle()                           //
//
// Wyświetla ekran bezczynności.                            // Displays the idle screen.                     //
//
//==============================================================

void Display::showIdle()
{
    //----------------------------------------------------------
    // Wymuś natychmiastowe narysowanie aktualnej godziny.      // Force the current time to be drawn immediately. //
    //----------------------------------------------------------

    lastIdleMinute = -1;

    updateIdle();
}

//==============================================================
// Funkcja clearScreen()                                    // Function clearScreen()                        //
//
// Całkowicie czyści ekran e-paper.                         // Completely clears the e-paper display.        //
//
// Po wykonaniu funkcji wyświetlacz pozostaje biały.        // After the function completes, the display remains white. //
//==============================================================

void Display::clearScreen()
{
    //----------------------------------------------------------
    // Ustawienie całego ekranu jako obszaru odświeżania.       // Set the entire screen as the refresh area.    //
    //----------------------------------------------------------

    epd.setFullWindow();


    //----------------------------------------------------------
    // Rozpoczęcie pełnego odświeżania.                         // Start a full refresh.                         //
    //----------------------------------------------------------

    epd.firstPage();

    do
    {
        //------------------------------------------------------
        // Wypełnienie całego ekranu kolorem białym.                // Fill the entire screen with white.            //
        //------------------------------------------------------

        epd.fillScreen(GxEPD_WHITE);

    }
    while (epd.nextPage());
}

//==============================================================
// Funkcja updateIdle()                                     // Function updateIdle()                         //
//
// Sprawdza aktualny czas RTC.                              // Checks the current RTC time.                  //
//
// Ekran e-paper jest odświeżany tylko wtedy, gdy zmieniła się // The e-paper screen is refreshed only when the minute changes. //
// minuta. Dzięki temu nie wykonujemy niepotrzebnych odświeżeń. // This prevents unnecessary refreshes.          //
//==============================================================

void Display::updateIdle()
{
    datetime_t now;

    //----------------------------------------------------------
    // Pobranie aktualnego czasu z RTC.                         // Read the current time from the RTC.           //
    //----------------------------------------------------------

    rtc_get_datetime(&now);

    //----------------------------------------------------------
    // Jeżeli minuta się nie zmieniła, nic nie robimy.          // If the minute has not changed, do nothing.    //
    //----------------------------------------------------------

    if (now.min == lastIdleMinute)
        return;

    //----------------------------------------------------------
    // Zapamiętanie aktualnej minuty.                           // Remember the current minute.                  //
    //----------------------------------------------------------

    lastIdleMinute = now.min;

    //----------------------------------------------------------
    // Odświeżenie całego ekranu Idle.                          // Refresh the entire Idle screen.               //
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
// Funkcja drawScrollingText()                              // Function drawScrollingText()                  //
//
// Rysuje tekst z uwzględnieniem przewijania.               // Draws text using page-based scrolling.        //
//==============================================================

void Display::drawScrollingText(const char* text,
                                ScrollState& scroll,
                                int x,
                                int y,
                                int width)
{
    //----------------------------------------------------------
    // Wyczyść obszar tekstu.                                   // Clear the text area.                          //
    //----------------------------------------------------------

   //----------------------------------------------------------
// Obliczenie wysokości aktualnie ustawionej czcionki.      // Calculate the height of the currently selected font. //
//
// Każda sekcja interfejsu (tytuł, wykonawca, nagłówek)     // Each interface section (title, artist, header) //
// może korzystać z innej czcionki. Z tego powodu wysokość  // may use a different font. Therefore, the height of the //
// czyszczonego obszaru nie powinna być wpisana "na sztywno", // cleared area should not be hard-coded,        //
// lecz wyznaczana na podstawie aktualnie ustawionej czcionki. // but determined from the currently selected font. //
//----------------------------------------------------------

int16_t x1;
int16_t y1;

uint16_t textWidth;
uint16_t textHeight;

// Pomiar przykładowego znaku.                              // Measure a sample character.                   //
// Litera "M" posiada zwykle największą wysokość.           // The letter "M" usually has the greatest height. //
epd.getTextBounds(
    "M",
    0,
    0,
    &x1,
    &y1,
    &textWidth,
    &textHeight);

//----------------------------------------------------------
// Wyczyść cały obszar zajmowany przez tekst.               // Clear the entire area occupied by the text.   //
//
// Dodawany jest niewielki margines (2 px), aby usunąć      // A small margin (2 px) is added to remove      //
// ewentualne pozostałości po poprzednim rysowaniu.         // any remnants of the previous drawing.         //
//----------------------------------------------------------

epd.fillRect(
    x,
    y - textHeight,
    width,
    textHeight + 2,
    GxEPD_WHITE);

    //----------------------------------------------------------
    // Ustaw pozycję kursora.                                   // Set the cursor position.                      //
    //----------------------------------------------------------

    //----------------------------------------------------------
    // Wyświetlenie tekstu zawsze w stałej pozycji.             // Always display the text at a fixed position.  //
    //----------------------------------------------------------
     epd.setCursor(x, y);

   
    //----------------------------------------------------------
    // Narysuj tekst.                                           // Draw the text.                                //
    //----------------------------------------------------------
    epd.setTextWrap(false);
    printPL(epd, getVisibleText(text, scroll, width).c_str());
}


//==============================================================
// Funkcja update()                                         // Function update()                             //
//
// Aktualizuje zawartość wyświetlacza.                      // Updates the display contents.                 //
//
// Parametry:                                               // Parameters:                                   //
//
// player                                                   // player                                        //
// Aktualny stan odtwarzacza.                               // Current player state.                         //
//
// changes                                                  // changes                                       //
// Zestaw flag określających wykryte zmiany.                // Set of flags indicating detected changes.     //
//
//==============================================================

void Display::update(const PlayerState& player,
                     ChangeFlags changes)

{

    //==========================================================
    // Obsługa trybu standby.                                   // Standby mode handling.                        //
    //
    // SYS:STANDBY powoduje wyczyszczenie całego wyświetlacza.  // SYS:STANDBY causes the entire display to be cleared. //
    //
    // W trybie standby ignorujemy pozostałe zmiany stanu       // In standby mode, other player state changes are ignored. //
    // odtwarzacza. Dzięki temu komunikaty takie jak MUT, VOL,  // This prevents messages such as MUT, VOL,      //
    // ELP itd. nie spowodują ponownego narysowania ekranu.     // ELP, etc. from causing the screen to be redrawn. //
    //==========================================================

    if (player.standby)
    {
        //------------------------------------------------------
        // Jeżeli właśnie weszliśmy w standby, wyczyść ekran.       // If we have just entered standby, clear the screen. //
        //------------------------------------------------------

        if ((changes & ChangeFlags::Standby)
            != ChangeFlags::None)
        {
            clearScreen();

            //--------------------------------------------------
            // Zapamiętujemy stan ekranu.                               // Remember the current screen state.            //
            //
            // Ekran nie jest ani odtwarzaczem, ani zegarem.            // The screen is neither the player screen nor the clock screen. //
            //--------------------------------------------------

            idleScreenActive = false;
        }

        //------------------------------------------------------
        // W standby niczego więcej nie rysujemy.                   // Nothing else is drawn in standby.             //
        //------------------------------------------------------

        return;
    }



//==========================================================
// Wyjście z trybu standby.                                 // Exit standby mode.                            //
//
// Po SYS:ON nie rysujemy jeszcze ekranu na podstawie       // After SYS:ON, the screen is not yet drawn using the //
// starego PlayerState.                                     // old PlayerState.                              //
//
// Zamiast tego pozwalamy głównej pętli ponownie pobrać     // Instead, the main loop is allowed to fetch the //
// aktualne źródło z Up2Stream.                             // current source from Up2Stream.                //
//==========================================================

if ((changes & ChangeFlags::Standby)
    != ChangeFlags::None)
{
    if (!player.standby)
    {
        //------------------------------------------------------//-----------------------------------------------//
// Urządzenie wróciło do normalnej pracy.                   // The device has returned to normal operation.  //
        //------------------------------------------------------//-----------------------------------------------//

        idleScreenActive = false;

        lastActivityMillis = millis();

        //------------------------------------------------------
        // Nie wyświetlamy jeszcze ekranu.                          // Do not display the screen yet.                //
        //
        // Aktualny stan źródła zostanie pobrany przez:             // The current source state will be obtained through: //
        //
        // SRC;                                                     // SRC;                                          //
        //
        //------------------------------------------------------

        //------------------------------------------------------
        // UWAGA:                                                   // NOTE:                                         //
        // Brak return.                                             // No return.                                    //
        //
        // Dalsza część Display::update() musi mieć możliwość       // The rest of Display::update() must be able to //
        // obsłużenia SRC/VND i odświeżenia ekranu.                 // handle SRC/VND and refresh the screen.        //
        //------------------------------------------------------
    }
}

    //----------------------------------------------------------
    // Obsługa przełączania pomiędzy ekranem odtwarzacza        // Handle switching between the player screen    //
    // a ekranem zegara.                                        // and the clock screen.                         //
    //
    // Zasada działania:                                        // Operating principle:                          //
    //
    // PLAY:                                                    // PLAY:                                         //
    // zawsze ekran odtwarzacza.                                // always show the player screen.                //
    //
    // PAUSE / STOP:                                            // PAUSE / STOP:                                 //
    // ekran odtwarzacza pozostaje przez IDLE_TIMEOUT,          // the player screen remains visible for IDLE_TIMEOUT, //
    // następnie przełączamy się na ekran zegara.               // then switch to the clock screen.              //
    //
    // PLAY podczas wyświetlania zegara:                        // PLAY while the clock is displayed:            //
    // natychmiast wracamy do ekranu odtwarzacza.               // immediately return to the player screen.      //
    //
    // Aktualizacja zegara odbywa się przez updateIdle().       // The clock is updated through updateIdle().    //
    //----------------------------------------------------------


    //----------------------------------------------------------
    // Sprawdź, czy zmienił się stan PLAY / PAUSE.              // Check whether the PLAY / PAUSE state has changed. //
    //
    // Jest to podstawowy sygnał sterujący przełączaniem        // This is the primary control signal for switching //
    // pomiędzy ekranami.                                       // between the screens.                          //
    //----------------------------------------------------------

    bool playStateChanged =
        (changes & ChangeFlags::PlayState)
        != ChangeFlags::None;


    //----------------------------------------------------------
    // Jeżeli zmienił się stan odtwarzania, zapamiętaj moment   // If the playback state changed, remember the time //
    // tej zmiany.                                              // of that change.                               //
    //
    // Jest to początek odliczania IDLE_TIMEOUT po przejściu    // This is the start of the IDLE_TIMEOUT countdown after //
    // z PLAY do PAUSE.                                         // switching from PLAY to PAUSE.                 //
    //----------------------------------------------------------

    if (playStateChanged)
    {
        lastActivityMillis = millis();
    }


    //----------------------------------------------------------
    // PLAY                                                     // PLAY                                          //
    //
    // Podczas odtwarzania zawsze pokazujemy ekran odtwarzacza. // During playback, always show the player screen. //
    //----------------------------------------------------------

    if (player.playing)
    {
        //------------------------------------------------------
        // Jeżeli zegar był aktualnie wyświetlany, natychmiast      // If the clock is currently displayed, immediately //
        // wracamy do ekranu odtwarzacza.                           // return to the player screen.                  //
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
        // Jeżeli już jesteśmy na ekranie odtwarzacza, niczego      // If we are already on the player screen, nothing //
        // nie zmieniamy w związku z mechanizmem Idle.              // is changed by the Idle mechanism.             //
        //------------------------------------------------------
    }


    //----------------------------------------------------------
    // PAUSE / STOP                                             // PAUSE / STOP                                  //
    //----------------------------------------------------------

    else
    {
        //------------------------------------------------------
        // Jeżeli zegar jest już aktywny, pozostawiamy go           // If the clock is already active, keep it active //
        // aktywnego, ale aktualizujemy jego zawartość z RTC.       // but update its contents from the RTC.         //
        //------------------------------------------------------

        if (idleScreenActive)
        {
            updateIdle();

            return;
        }


        //------------------------------------------------------
        // Jesteśmy jeszcze na ekranie odtwarzacza.                 // We are still on the player screen.            //
        //
        // Sprawdź, czy minął czas bezczynności.                    // Check whether the idle time has elapsed.      //
        //------------------------------------------------------

        if (millis() - lastActivityMillis >= IDLE_TIMEOUT)
        {
            //--------------------------------------------------
            // Przełączenie na ekran zegara.                            // Switch to the clock screen.                   //
            //--------------------------------------------------

            idleScreenActive = true;

            Serial.println(
                "DISPLAY: PAUSE -> EKRAN ZEGARA");

            showIdle();

            return;
        }
    }


    //----------------------------------------------------------
    // Od tego miejsca pozostaje dotychczasowa obsługa ekranu   // From this point, the existing player screen handling remains: //
    // odtwarzacza:                                             // of the player screen:                         //
    //
    // - inicjalizacja przewijania tytułu,                      // - initialize title page switching,            //
    // - inicjalizacja przewijania wykonawcy,                   // - initialize artist page switching,           //
    // - aktualizacja stron tekstu,                             // - update text pages,                          //
    // - częściowe odświeżenie ekranu.                          // - perform a partial screen refresh.           //
    //----------------------------------------------------------

    //----------------------------------------------------------
    // Jeżeli zmienił się tytuł utworu,                         // If the track title changed,                   //
    // zainicjalizuj przewijanie od początku.                   // initialize page switching from the beginning. //
    //----------------------------------------------------------


   if ((changes & ChangeFlags::Title) != ChangeFlags::None)
   {
    
    //----------------------------------------------------------
    // Inicjalizacja przewijania nazwy wykonawcy.               // Initialize page switching for the artist name. //
    //
    // Pomiar wykonywany jest z wykorzystaniem czcionki         // Measurement is performed using the font       //
    // przeznaczonej dla wykonawcy.                             // intended for the artist.                      //
    //----------------------------------------------------------

    initScroll(
        titleScroll,
        player.title.c_str(),
        TEXT_WIDTH,
        &FONT_TITLE);

              
   }

   //----------------------------------------------------------
   // Jeżeli zmienił się wykonawca,                            // If the artist changed,                        //
   // zainicjalizuj przewijanie od początku.                   // initialize page switching from the beginning. //
   //----------------------------------------------------------

   if ((changes & ChangeFlags::Artist) != ChangeFlags::None)
   {
    //----------------------------------------------------------
    // Inicjalizacja przewijania nazwy wykonawcy.               // Initialize page switching for the artist name. //
    //
    // Pomiar wykonywany jest z wykorzystaniem czcionki         // Measurement is performed using the font       //
    // przeznaczonej dla wykonawcy.                             // intended for the artist.                      //
    //----------------------------------------------------------

    initScroll(
        artistScroll,
        player.artist.c_str(),
        TEXT_WIDTH,
        &FONT_ARTIST);
   }

   //----------------------------------------------------------
   // Aktualizacja pozycji przewijania.                        // Update the page switching position.           //
   //----------------------------------------------------------

    bool titleMoved  = updateScroll(titleScroll);
    bool artistMoved = updateScroll(artistScroll);


 //----------------------------------------------------------
 // Jeżeli wykryto jakąkolwiek zmianę,                       // If any change was detected,                   //
 // wykonaj częściowe odświeżenie.                           // perform a partial refresh.                    //
 //
 // Pełne odświeżenie wykonywane jest tylko po uruchomieniu  // A full refresh is performed only when the device starts //
 // urządzenia.                                              // of the device.                                //
 //----------------------------------------------------------


 if (changes != ChangeFlags::None ||
    titleMoved ||
    artistMoved)
{
    //----------------------------------------------------------
    // Diagnostyka aktualizacji ekranu.                         // Display update diagnostics.                   //
    //
    // Sprawdzamy, czy każda zmiana głośności dociera           // Check whether every volume change reaches     //
    // do modułu Display.                                       // the Display module.                           //
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
// Funkcja drawPlayerScreen()                               // Function drawPlayerScreen()                   //
//
// Rysuje kompletną zawartość ekranu odtwarzacza.           // Draws the complete player screen.             //
//
// Funkcja odpowiada wyłącznie za rysowanie elementów       // The function is responsible only for drawing user interface //
// interfejsu użytkownika.                                  // elements.                                     //
//
// Nie rozpoczyna ani nie kończy odświeżania wyświetlacza.  // It does not start or finish a display refresh. //
//
//==============================================================

void Display::drawPlayerScreen(const PlayerState& player)
{
//----------------------------------------------------------
// Nagłówek z nazwą źródła + separator (linia oddzielająca). // Header with the source name + separator (dividing line). //
//----------------------------------------------------------

//----------------------------------------------------------
// Przygotowanie tekstu źródła.                             // Prepare the source text.                      //
//
// Dla źródła NET, jeżeli otrzymaliśmy VND, wyświetlamy:    // For the NET source, if VND was received, display: //
//
// NET: spotify                                             // NET: spotify                                  //
//
// Jeżeli VND nie zostało odebrane:                         // If VND was not received:                      //
//
// NET                                                      // NET                                           //
//
// Dla pozostałych źródeł wyświetlamy wyłącznie SRC.        // For other sources, display only SRC.          //
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
// Nagłówek z nazwą źródła + separator.                     // Header with the source name + separator.      //
//----------------------------------------------------------

drawHeaderArea(
    sourceText.c_str());
    
    //----------------------------------------------------------
    // Informacje o aktualnie odtwarzanym utworze.              // Information about the currently playing track. //
    //----------------------------------------------------------

   drawTrackInfo(
    player.title.c_str(),
    player.artist.c_str());

//----------------------------------------------------------
// Pasek postępu odtwarzania.                               // Playback progress bar.                        //
//
// Przekazujemy również aktualny stan odtwarzania,          // The current playback state is also passed     //
// aby ikona mogła pokazać:                                 // so that the icon can show:                    //
//
// PLAY  - podczas odtwarzania,                             // PLAY  - during playback,                      //
// PAUSE - podczas pauzy.                                   // PAUSE - while paused.                         //
//----------------------------------------------------------

   drawPlaybackBar(
    player.currentTime,
    player.totalTime,
    player.progress,
    player.playing);

    //----------------------------------------------------------
    // Aktualny poziom głośności.                               // Current volume level.                         //
    //----------------------------------------------------------

    drawVolume(player.volume);
}

//==============================================================
// Funkcja drawIdleScreen()                                 // Function drawIdleScreen()                     //
//
// Rysuje ekran bezczynności.                               // Draws the idle screen.                        //
//
// Na obecnym etapie wyświetlane są przykładowa data,       // At the current stage, the screen displays the actual date, //
// przykładowa godzina oraz podpis urządzenia.              // the actual time and the device footer.        //
//==============================================================

void Display::drawIdleScreen()
{
    //----------------------------------------------------------
    // Wyczyść cały ekran.                                      // Clear the entire screen.                      //
    //----------------------------------------------------------

    epd.fillScreen(GxEPD_WHITE);

        //----------------------------------------------------------
    // Pobranie aktualnej daty i godziny z RTC.                 // Read the current date and time from the RTC.  //
    //----------------------------------------------------------

    datetime_t now;

    rtc_get_datetime(&now);

    //----------------------------------------------------------
    // Data.                                                    // Date.                                         //
    //
    // Format:                                                  // Format:                                       //
    //
    // DD.MM.RRRR                                               // DD.MM.YYYY                                    //
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
    // Godzina.                                                 // Time.                                         //
    //----------------------------------------------------------

//----------------------------------------------------------
// Wyświetlenie dużego zegara.                              // Display the large clock.                      //
//
// Pozycja pozioma wyznaczana jest automatycznie,           // The horizontal position is calculated automatically, //
// dzięki czemu każda godzina będzie idealnie               // so every time value is perfectly              //
// wyśrodkowana.                                            // centered.                                     //
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
    // Linia oddzielająca podpis.                               // Separator line above the footer.              //
    //----------------------------------------------------------

    epd.drawLine(
    0,
    IDLE_LINE_Y,
    epd.width() - 1,
    IDLE_LINE_Y,
    GxEPD_BLACK);

    //----------------------------------------------------------
    // Podpis urządzenia.                                       // Device footer.                                //
    //----------------------------------------------------------

    epd.setFont(&FONT_STATUS);

    epd.setCursor(
    IDLE_FOOTER_X,
    IDLE_FOOTER_Y);

    epd.print("MEANDRY TECHNIKI");
}


//==============================================================
// Funkcja drawSourceInfo()                                 // Function drawSourceInfo()                     //
//
// Rysuje informacje o aktualnym źródle dźwięku.            // Draws information about the current audio source. //
//
// Parametry:                                               // Parameters:                                   //
//
// source                                                   // source                                        //
// Nazwa aktualnego źródła.                                 // Name of the current source.                   //
//
// Zwraca:                                                  // Returns:                                      //
//
// nic                                                      // nothing                                       //
//
//==============================================================

void Display::drawSourceInfo(const char* source)

{
    //----------------------------------------------------------
    // Wyświetlenie nazwy aktualnego źródła.                    // Display the name of the current source.       //
    //
    // Zrezygnowano z ikon źródeł (Spotify, Bluetooth, AirPlay, // Source icons (Spotify, Bluetooth, AirPlay,    //
    // NAS, USB itd.), ponieważ na wyświetlaczu e-paper         // NAS, USB, etc.) were removed because on the e-paper display //
    // nie zapewniają wystarczającej czytelności.               // they do not provide sufficient readability.   //
    //----------------------------------------------------------

    drawSourceName(source);
}


//==============================================================
// Funkcja drawSourceName()                                 // Function drawSourceName()                     //
//
// Rysuje nazwę aktualnego źródła dźwięku.                  // Draws the name of the current audio source.   //
// Parametry:                                               // Parameters:                                   //
// source                                                   // source                                        //
// Nazwa źródła.                                            // Source name.                                  //
// Zwraca:                                                  // Returns:                                      //
// nic                                                      // nothing                                       //
//
//==============================================================

void Display::drawSourceName(const char* source)
{
    drawHeader(source);
}

//==============================================================
// Rysowanie nagłówka z nazwą źródła dźwięku.               // Draw the header with the audio source name.   //
//==============================================================

void Display::drawHeader(const char* source)
{
    epd.setFont(&FONT_STATUS);
//----------------------------------------------------------
// Nazwa źródła rozpoczyna się od lewego marginesu.         // The source name starts at the left margin.    //
//
// Zrezygnowano z ikon źródeł, dlatego nie jest już         // Source icons were removed, so no space is reserved //
// rezerwowane miejsce na ich wyświetlenie.                 // for displaying them.                          //
//----------------------------------------------------------
    epd.setCursor(
    MARGIN_X,
    HEADER_Y);
    printPL(epd, source);
}

//==============================================================
// Funkcja drawHeaderArea()                                 // Function drawHeaderArea()                     //
//
// Rysuje cały obszar nagłówka.                             // Draws the entire header area.                 //
//
// Parametry:                                               // Parameters:                                   //
//
// source                                                   // source                                        //
// Nazwa aktualnego źródła dźwięku.                         // Name of the current audio source.             //
//
// Zwraca:                                                  // Returns:                                      //
//
// nic                                                      // nothing                                       //
//
//==============================================================

void Display::drawHeaderArea(const char* source)
{
    //----------------------------------------------------------
    // Rysowanie nagłówka.                                      // Draw the header.                              //
    //----------------------------------------------------------

    drawSourceInfo(source);

    //----------------------------------------------------------
    // Linia oddzielająca nagłówek od pozostałej części ekranu. // Separator line between the header and the rest of the screen. //
    //----------------------------------------------------------

    drawHeaderSeparator();
}

//==============================================================
// Funkcja drawTrackInfo()                                  // Function drawTrackInfo()                      //
//
// Rysuje informacje o aktualnie odtwarzanym utworze.       // Draws information about the currently playing track. //
//
// Parametry:                                               // Parameters:                                   //
//
// title                                                    // title                                         //
// Tytuł utworu.                                            // Track title.                                  //
//
// artist                                                   // artist                                        //
// Wykonawca.                                               // Artist.                                       //
//
// Zwraca:                                                  // Returns:                                      //
//
// nic                                                      // nothing                                       //
//
//==============================================================

void Display::drawTrackInfo(const char* title,
                            const char* artist)
{
    drawTitle(title);

    drawArtist(artist);
}

//==============================================================
// Rysowanie tytułu aktualnie odtwarzanego utworu.          // Draw the title of the currently playing track. //
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
// Obliczenie szerokości tekstu.                            // Calculate the text width.                     //
//
// Funkcja chwilowo ustawia przekazaną czcionkę,            // The function temporarily sets the supplied font, //
// oblicza szerokość napisu, a następnie zwraca wynik.      // calculates the text width, and returns the result. //
//
// Dzięki temu szerokość zawsze odpowiada czcionce,         // This ensures that the width always corresponds to the font //
// która będzie użyta podczas rysowania.                    // that will be used for drawing.                //
//--------------------------------------------------------------
uint16_t Display::measureTextWidth(
    const String& text,
    const GFXfont* font)
{
    //----------------------------------------------------------
    // Ustawienie czcionki używanej do pomiaru.                 // Set the font used for measurement.            //
    //----------------------------------------------------------

    epd.setFont(font);

    //----------------------------------------------------------
    // Wyznaczenie prostokąta ograniczającego napis.            // Determine the text bounding rectangle.        //
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
// Funkcja calculateCenteredX()                             // Function calculateCenteredX()                 //
//
// Oblicza współrzędną X umożliwiającą wyśrodkowanie napisu // Calculates the X coordinate required to center the text //
// w zadanym obszarze.                                      // within the specified area.                    //
//==============================================================

int Display::calculateCenteredX(
    const String& text,
    const GFXfont* font,
    int areaX,
    int areaWidth)
{
    //----------------------------------------------------------
    // Obliczenie szerokości napisu.                            // Calculate the text width.                     //
    //----------------------------------------------------------

    uint16_t width =
        measureTextWidth(text, font);

    //----------------------------------------------------------
    // Wyznaczenie współrzędnej X.                              // Determine the X coordinate.                   //
    //----------------------------------------------------------

    return areaX + (areaWidth - width) / 2;
}

//==============================================================
// Funkcja scorePage()                                      // Function scorePage()                          //
//
// Oblicza ocenę jakości podziału tekstu.                   // Calculates the quality score of a text page split. //
//
// Im lepiej wykorzystana jest szerokość strony,            // The better the page width is utilized,        //
// tym wyższy wynik.                                        // the higher the score.                         //
//
// W kolejnych etapach funkcja zostanie rozszerzona o:      // In later stages, the function will be extended with: //
// - karę za pozostawienie bardzo krótkiej następnej strony, // - a penalty for leaving a very short next page, //
// - karę za rozpoczynanie strony od krótkich spójników,    // - a penalty for starting a page with short conjunctions, //
// - premię za bardziej naturalny podział tekstu.           // - a bonus for a more natural text split.      //
//
// Parametry:                                               // Parameters:                                   //
//
// page                                                     // page                                          //
// Kandydat na bieżącą stronę.                              // Candidate for the current page.               //
//
// remaining                                                // remaining                                     //
// Pozostała część tekstu.                                  // Remaining part of the text.                   //
//
// areaWidth                                                // areaWidth                                     //
// Maksymalna szerokość strony w pikselach.                 // Maximum page width in pixels.                 //
//
// Zwraca:                                                  // Returns:                                      //
//
// Wynik oceny. Im większa wartość, tym lepszy podział.     // Score. The higher the value, the better the split. //
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
// Rysowanie nazwy wykonawcy.                               // Draw the artist name.                         //
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
// Rysowanie paska postępu odtwarzania                      // Draw the playback progress bar                //
//
// Funkcja rysuje:                                          // The function draws:                           //
// - ikonę Play,                                            // - the Play icon,                              //
// - aktualny czas,                                         // - the current time,                           //
// - pasek postępu,                                         // - the progress bar,                           //
// - znacznik aktualnej pozycji,                            // - the current position marker,                //
// - całkowity czas utworu.                                 // - the total track time.                       //
//==============================================================

void Display::drawPlaybackBar(
    const char* currentTime,
    const char* totalTime,
    int progress,
    bool playing)
{

    //----------------------------------------------------------
    // Diagnostyka danych przekazywanych do paska odtwarzania.  // Diagnostics for data passed to the playback bar. //
    //----------------------------------------------------------

    Serial.print("DRAW PLAYBACK  ");

    Serial.print(currentTime);

    Serial.print(" / ");

    Serial.print(totalTime);

    Serial.print("  PROGRESS=");

    Serial.println(progress);


//----------------------------------------------------------
// Ikona stanu odtwarzania.                                 // Playback state icon.                          //
//
// PLAY  - gdy odtwarzanie jest aktywne.                    // PLAY  - when playback is active.              //
// PAUSE - gdy odtwarzanie jest wstrzymane.                 // PAUSE - when playback is paused.              //
//----------------------------------------------------------

epd.setFont(&FONT_TIME);

epd.setCursor(CURRENT_TIME_X, TIME_Y);

if (playing)
{
    //------------------------------------------------------
    // Ikona PLAY - trójkąt.                                    // PLAY icon - triangle.                         //
    //------------------------------------------------------

    printPL(epd, "\x92");
}
else
{
    //------------------------------------------------------
    // Ikona PAUSE - dwie pionowe kreski.                       // PAUSE icon - two vertical bars.               //
    //------------------------------------------------------

    printPL(epd, "\x93");
}

epd.print("  ");
printPL(epd, currentTime);

    //----------------------------------------------------------
    // Parametry paska postępu.                                 // Progress bar parameters.                      //
    //----------------------------------------------------------

    const int barX = BAR_X;
    const int barY = BAR_Y;
    const int barWidth = epd.width() - 1;

    //----------------------------------------------------------
// Obliczenie szerokości odtworzonej części paska.          // Calculate the width of the played portion of the bar. //
// Parametr progress określa postęp odtwarzania             // The progress parameter specifies playback progress //
// w procentach (0...100).                                  // as a percentage (0...100).                    //
//----------------------------------------------------------

int playedWidth = (barWidth * progress) / 100;

//----------------------------------------------------------
// Obrys całego paska.                                      // Outline of the entire bar.                    //
//----------------------------------------------------------

epd.drawRect(barX,
             barY - 3,
             barWidth,
             4,
             GxEPD_BLACK);

//----------------------------------------------------------
// Wypełnienie odtworzonej części.                          // Fill the played portion.                      //
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
    // Całkowity czas utworu.                                   // Total track time.                             //
    //----------------------------------------------------------

    epd.setCursor(TOTAL_TIME_X, TIME_Y);
    epd.print(totalTime);
}

//==============================================================
// Funkcja drawVolume()                                     // Function drawVolume()                         //
//
// Rysuje aktualny poziom głośności.                        // Draws the current volume level.               //
//
// Parametry:                                               // Parameters:                                   //
//
// volume                                                   // volume                                        //
// Aktualny poziom głośności (0...100%).                    // Current volume level (0...100%).              //
//
//==============================================================

void Display::drawVolume(int volume)
{
    //----------------------------------------------------------
    // Ustawienie pozycji kursora.                              // Set the cursor position.                      //
    //----------------------------------------------------------
    epd.setFont(&FONT_VOLUME);

    epd.setCursor(VOLUME_X, VOLUME_Y);

    //----------------------------------------------------------
    // Wyświetlenie poziomu głośności.                          // Display the volume level.                     //
    //----------------------------------------------------------

    epd.print("VOL ");

    epd.print(volume);

    epd.print("%");
}

//==============================================================
// Rysowanie poziomej linii oddzielającej sekcje ekranu.    // Draw a horizontal line separating screen sections. //
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
// Funkcja drawHeaderSeparator()                            // Function drawHeaderSeparator()                //
//
// Rysuje linię oddzielającą nagłówek od pozostałej części  // Draws the line separating the header from the rest of //
// ekranu.                                                  // the screen.                                   //
//
// Parametry:                                               // Parameters:                                   //
//
// brak                                                     // none                                          //
//
// Zwraca:                                                  // Returns:                                      //
//
// nic                                                      // nothing                                       //
//
//==============================================================

void Display::drawHeaderSeparator()
{
    drawSeparator(HEADER_LINE);
}

//==============================================================
// Zwrócenie aktualnie wyświetlanej strony tekstu           // Return the currently displayed text page      //
//==============================================================
String Display::getVisibleText(
    const String& text,
    ScrollState& scroll,
    int maxWidth)
{
    (void)text;
    (void)maxWidth;

    //----------------------------------------------------------
    // Brak przygotowanych stron.                               // No prepared pages.                            //
    //----------------------------------------------------------
    if (scroll.pageCount == 0)
        return "";

    //----------------------------------------------------------
    // Zabezpieczenie przed błędnym indeksem.                   // Protection against an invalid index.          //
    //----------------------------------------------------------
    if (scroll.currentPage >= scroll.pageCount)
        scroll.currentPage = 0;

    //----------------------------------------------------------
    // Zwrócenie aktualnej strony.                              // Return the current page.                      //
    //----------------------------------------------------------
    return scroll.pages[scroll.currentPage];
}
