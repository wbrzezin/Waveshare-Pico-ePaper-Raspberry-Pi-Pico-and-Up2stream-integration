//==============================================================//-----------------------------------------------//
// Projekt : UP2Stream Display                              // Project : UP2Stream Display                   //
// Plik    : Display.h                                      // File    : Display.h                           //
// Autor   : Waldemar Brzeziński                            // Author  : Waldemar Brzeziński                 //
//                                                          //                                               //
// Opis:                                                    // Description:                                  //
//                                                          //                                               //
// Deklaracja klasy Display.                               // Declaration of the Display class.            //
//==============================================================//-----------------------------------------------//


#ifndef DISPLAY_H
#define DISPLAY_H


#include <Arduino.h>


//--------------------------------------------------------------//-----------------------------------------------//
// Definicja struktury GFXfont wykorzystywanej przez bibliotekę // Definition of the GFXfont structure used by //
// Adafruit_GFX.                                              // the Adafruit_GFX library.                    //
//--------------------------------------------------------------//-----------------------------------------------//

#include <Adafruit_GFX.h>


#include "PlayerState.h"
#include "ChangeFlags.h"
#include "ScrollState.h"
#include "PolishTime.h"



//--------------------------------------------------------------//-----------------------------------------------//
// Klasa Display                                               // Display class                                //
//                                                              //                                               //
// Odpowiada za obsługę wyświetlacza e-paper oraz rysowanie     // Responsible for controlling the e-paper      //
// interfejsu użytkownika odtwarzacza UP2Stream.               // display and drawing the UP2Stream player      //
//                                                              // user interface.                              //
//                                                              //                                               //
// Zadania klasy:                                               // Class responsibilities:                      //
// - inicjalizacja wyświetlacza,                               // - display initialization,                     //
// - wyświetlenie ekranu startowego,                           // - displaying the startup screen,             //
// - rysowanie ekranu odtwarzacza,                             // - drawing the player screen,                  //
// - rysowanie ekranu bezczynności i zegara,                   // - drawing the idle and clock screens,         //
// - aktualizacja elementów dynamicznych,                      // - updating dynamic elements,                  //
// - obsługa przewijania długich nazw utworów i wykonawców.    // - scrolling long track and artist names.      //
//--------------------------------------------------------------//-----------------------------------------------//

class Display
{
public:


    //==========================================================//-----------------------------------------------//
    // Funkcje publiczne                                        // Public functions                             //
    // Wywoływane przez główny program.                         // Called by the main program.                  //
    //==========================================================//-----------------------------------------------//


    //----------------------------------------------------------//-----------------------------------------------//
    // Inicjalizacja wyświetlacza.                             // Initialize the display.                     //
    //                                                          //                                               //
    // Funkcja konfiguruje interfejs SPI, uruchamia sterownik   // The function configures the SPI interface,    //
// wyświetlacza oraz wyświetla ekran startowy.                 // initializes the display driver and displays   //
//                                                              // the startup screen.                           //
//                                                              //                                               //
// Zwraca:                                                     // Returns:                                      //
// true - inicjalizacja zakończyła się powodzeniem.            // true - initialization completed successfully.//
//----------------------------------------------------------//-----------------------------------------------//

    bool begin();


    //----------------------------------------------------------//-----------------------------------------------//
    // Wyświetlenie ekranu startowego.                         // Display the startup screen.                  //
    //                                                          //                                               //
// Funkcja wykonywana jednorazowo podczas uruchamiania         // The function is executed once during device   //
// urządzenia.                                                // startup.                                      //
//----------------------------------------------------------//-----------------------------------------------//

    void splash();


    //----------------------------------------------------------//-----------------------------------------------//
    // Wyświetlenie głównego ekranu odtwarzacza.               // Display the main player screen.              //
    //                                                          //                                               //
// Parametr:                                                  // Parameter:                                    //
// player - komplet informacji o aktualnym stanie             // player - complete information about the        //
//          odtwarzacza.                                      //          current player state.                 //
//----------------------------------------------------------//-----------------------------------------------//

    void showPlayer(const PlayerState& player);


    //==========================================================//-----------------------------------------------//
    // Ustawienie zegara RTC                                    // Set the RTC clock                            //
    //                                                          //                                               //
// Parametr:                                                  // Parameter:                                    //
//                                                              //                                               //
// time                                                       // time                                          //
//      Aktualny czas polski obliczony na podstawie odpowiedzi //      Current Polish local time calculated      //
//      TME z modułu UP2Stream.                                //      from the TME response received from       //
//                                                              //      the Up2Stream module.                     //
//==========================================================//-----------------------------------------------//

    void setRTC(
        const PolishTime& time);


    //----------------------------------------------------------//-----------------------------------------------//
    // Wyświetlenie ekranu bezczynności.                       // Display the idle screen.                     //
//                                                              //                                               //
// Funkcja wykonuje pełne odświeżenie wyświetlacza i rysuje    // The function performs a full display refresh  //
// ekran zawierający aktualną datę i czas.                     // and draws the screen containing the current   //
//                                                              // date and time.                                //
//----------------------------------------------------------//-----------------------------------------------//

    void showIdle();


    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualizacja zegara na ekranie bezczynności.             // Update the clock on the idle screen.         //
//                                                              //                                               //
// Ekran jest aktualizowany zgodnie z aktualnym czasem RTC.    // The screen is updated according to the current //
//                                                              // RTC time.                                     //
//----------------------------------------------------------//-----------------------------------------------//

    void updateIdle();


    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualizacja zawartości wyświetlacza.                    // Update the display contents.                 //
//                                                              //                                               //
// Funkcja analizuje przekazane informacje o zmianach i         // The function analyzes the supplied change      //
// odświeża odpowiednie elementy interfejsu.                   // information and refreshes the appropriate     //
//                                                              // interface elements.                           //
//                                                              //                                               //
// Parametry:                                                  // Parameters:                                   //
//                                                              //                                               //
// player                                                       // player                                        //
//      Aktualny stan odtwarzacza.                             //      Current player state.                    //
//                                                              //                                               //
// changes                                                      // changes                                       //
//      Flagi określające, które elementy uległy zmianie.      //      Flags indicating which elements have     //
//                                                              //      changed.                                  //
//----------------------------------------------------------//-----------------------------------------------//

    void update(const PlayerState& player,
                ChangeFlags changes);



    //----------------------------------------------------------//-----------------------------------------------//
    // Wyczyść cały ekran e-paper.                             // Clear the entire e-paper display.            //
    //                                                              //                                               //
    // Funkcja używana podczas przejścia modułu Up2Stream         // The function is used when the Up2Stream module //
    // w tryb standby.                                             // enters standby mode.                         //
    //----------------------------------------------------------//-----------------------------------------------//

    void clearScreen();


private:


    //==========================================================//-----------------------------------------------//
    // Funkcje odpowiedzialne za rysowanie interfejsu            // Functions responsible for drawing the user   //
// użytkownika.                                               // interface.                                   //
//==========================================================//-----------------------------------------------//

bool rtcSynchronized = false;


    //==========================================================//-----------------------------------------------//
    // Funkcja refreshFull()                                    // refreshFull() function                       //
//                                                              //                                               //
// Wykonuje pełne odświeżenie wyświetlacza.                    // Performs a full display refresh.              //
//==========================================================//-----------------------------------------------//

    void refreshFull(const PlayerState& player);


    //==========================================================//-----------------------------------------------//
    // Funkcja refreshPartial()                                 // refreshPartial() function                    //
//                                                              //                                               //
// Wykonuje odświeżenie wybranych elementów wyświetlacza       // Refreshes selected display elements based on  //
// na podstawie przekazanych flag zmian.                       // the supplied change flags.                   //
//                                                              //                                               //
// W zależności od aktualnej implementacji może wykorzystywać  // Depending on the current implementation, it   //
// częściowy obszar odświeżania e-paper.                       // may use a partial e-paper refresh area.      //
//==========================================================//-----------------------------------------------//

    void refreshPartial(const PlayerState& player,
                        ChangeFlags changes);


    //==========================================================//-----------------------------------------------//
    // Funkcja drawPlayerScreen()                              // drawPlayerScreen() function                  //
//                                                              //                                               //
// Rysuje kompletny ekran odtwarzacza.                         // Draws the complete player screen.             //
//                                                              //                                               //
// Funkcja jest wykorzystywana podczas pełnego odświeżania      // The function is used during a full display     //
// wyświetlacza e-paper.                                       // refresh.                                      //
//                                                              //                                               //
// Parametry:                                                  // Parameters:                                   //
//                                                              //                                               //
// player                                                       // player                                        //
//      Aktualny stan odtwarzacza.                             //      Current player state.                    //
//==========================================================//-----------------------------------------------//

    void drawPlayerScreen(const PlayerState& player);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie ekranu bezczynności.                          // Draw the idle screen.                        //
//                                                              //                                               //
// Ekran jest wyświetlany po dłuższym okresie braku aktywności // The screen is displayed after a longer period //
// użytkownika.                                                // of user inactivity.                           //
//                                                              //                                               //
// Zawiera aktualną datę i czas pobierane z RTC Pico.           // It contains the current date and time obtained //
//                                                              // from the Pico RTC.                            //
//----------------------------------------------------------//-----------------------------------------------//

    void drawIdleScreen();


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie nagłówka z nazwą źródła dźwięku.               // Draw the audio source header.                //
//----------------------------------------------------------//-----------------------------------------------//

    void drawHeader(const char* source);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie poziomej linii oddzielającej sekcje ekranu.    // Draw a horizontal line separating screen      //
//                                                              // sections.                                     //
//----------------------------------------------------------//-----------------------------------------------//

    void drawSeparator(int y);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie całego obszaru nagłówka.                       // Draw the complete header area.               //
//----------------------------------------------------------//-----------------------------------------------//

    void drawHeaderArea(const char* source);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie nazwy źródła dźwięku.                          // Draw the audio source name.                   //
//----------------------------------------------------------//-----------------------------------------------//

    void drawSourceName(const char* source);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie informacji o źródle dźwięku.                   // Draw the audio source information.            //
//----------------------------------------------------------//-----------------------------------------------//

    void drawSourceInfo(const char* source);


    //==========================================================//-----------------------------------------------//
    // Funkcja drawHeaderSeparator()                           // drawHeaderSeparator() function               //
//                                                              //                                               //
// Rysuje linię oddzielającą nagłówek od pozostałej części     // Draws the line separating the header from the //
// ekranu.                                                     // remaining screen area.                        //
//                                                              //                                               //
// Parametry: brak                                             // Parameters: none                              //
//                                                              //                                               //
// Zwraca: nic                                                 // Returns: nothing                              //
//==========================================================//-----------------------------------------------//

    void drawHeaderSeparator();


    //----------------------------------------------------------//-----------------------------------------------//
    // Obliczenie szerokości tekstu.                           // Calculate text width.                        //
//                                                              //                                               //
// Funkcja zwraca szerokość napisu dla wskazanej czcionki.     // The function returns the width of the text for //
// Dzięki temu wynik nie zależy od aktualnie ustawionej        // the specified font. This makes the result      //
// czcionki wyświetlacza.                                      // independent of the currently selected display //
//                                                              // font.                                         //
//----------------------------------------------------------//-----------------------------------------------//

    uint16_t measureTextWidth(
        const String& text,
        const GFXfont* font);


    //----------------------------------------------------------//-----------------------------------------------//
    // Obliczenie współrzędnej X zapewniającej wyśrodkowanie    // Calculate the X coordinate required to center //
// napisu.                                                     // the text.                                      //
//                                                              //                                               //
// Parametry:                                                  // Parameters:                                   //
//                                                              //                                               //
// text                                                         // text                                          //
//      Tekst do wyświetlenia.                                 //      Text to display.                          //
//                                                              //                                               //
// font                                                         // font                                          //
//      Czcionka użyta do obliczenia szerokości.               //      Font used to calculate the text width.   //
//                                                              //                                               //
// areaX                                                        // areaX                                         //
//      Lewa krawędź obszaru.                                  //      Left edge of the area.                    //
//                                                              //                                               //
// areaWidth                                                    // areaWidth                                     //
//      Szerokość obszaru.                                     //      Width of the area.                        //
//                                                              //                                               //
// Zwraca współrzędną X, od której należy rozpocząć            // Returns the X coordinate at which text should  //
// rysowanie tekstu.                                           // be drawn.                                     //
//----------------------------------------------------------//-----------------------------------------------//

    int calculateCenteredX(
        const String& text,
        const GFXfont* font,
        int areaX,
        int areaWidth);



    //----------------------------------------------------------//-----------------------------------------------//
    // Wyznaczenie fragmentu tekstu mieszczącego się w          // Determine the text fragment that fits within  //
// dostępnym obszarze.                                        // the available area.                           //
//----------------------------------------------------------//-----------------------------------------------//

    String getVisibleText(
        const String& text,
        ScrollState& scroll,
        int maxWidth);


    //----------------------------------------------------------//-----------------------------------------------//
    // Inicjalizacja przewijanego tekstu.                      // Initialize scrolling text.                  //
//                                                              //                                               //
// Parametry:                                                  // Parameters:                                   //
//                                                              //                                               //
// scroll                                                       // scroll                                        //
//      Struktura przechowująca stan przewijania.              //      Structure storing the scrolling state.   //
//                                                              //                                               //
// text                                                         // text                                          //
//      Tekst do wyświetlenia.                                 //      Text to display.                          //
//                                                              //                                               //
// areaWidth                                                    // areaWidth                                     //
//      Dostępna szerokość obszaru w pikselach.                //      Available area width in pixels.          //
//                                                              //                                               //
// font                                                         // font                                          //
//      Czcionka wykorzystywana zarówno do pomiaru szerokości, //      Font used both for width measurement and  //
//      jak i późniejszego wyświetlenia tekstu.                //      subsequent text rendering.               //
//----------------------------------------------------------//-----------------------------------------------//

    void initScroll(
        ScrollState& scroll,
        const char* text,
        int areaWidth,
        const GFXfont* font);


    //==========================================================//-----------------------------------------------//
    // Funkcja updateScroll()                                  // updateScroll() function                      //
//                                                              //                                               //
// Aktualizuje pozycję przewijanego tekstu.                    // Updates the position of scrolling text.       //
//==========================================================//-----------------------------------------------//

    bool updateScroll(ScrollState& scroll);


    //==========================================================//-----------------------------------------------//
    // Funkcja drawScrollingText()                             // drawScrollingText() function                 //
//                                                              //                                               //
// Rysuje tekst w zadanym obszarze.                            // Draws text within the specified area.         //
//==========================================================//-----------------------------------------------//

    void drawScrollingText(
        const char* text,
        ScrollState& scroll,
        int x,
        int y,
        int width);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie tytułu aktualnie odtwarzanego utworu.         // Draw the title of the currently playing track.//
//----------------------------------------------------------//-----------------------------------------------//

    void drawTitle(const char* title);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie nazwy wykonawcy.                              // Draw the artist name.                        //
//----------------------------------------------------------//-----------------------------------------------//

    void drawArtist(const char* artist);


    //==========================================================//-----------------------------------------------//
    // Funkcja drawTrackInfo()                                 // drawTrackInfo() function                     //
//                                                              //                                               //
// Rysuje informacje o aktualnie odtwarzanym utworze.          // Draws information about the currently playing //
//                                                              // track.                                        //
//                                                              //                                               //
// Parametry:                                                  // Parameters:                                   //
//                                                              //                                               //
// title                                                        // title                                         //
//      Tytuł utworu.                                          //      Track title.                              //
//                                                              //                                               //
// artist                                                       // artist                                        //
//      Wykonawca.                                             //      Artist name.                              //
//                                                              //                                               //
// Zwraca: nic                                                 // Returns: nothing                              //
//==========================================================//-----------------------------------------------//

    void drawTrackInfo(
        const char* title,
        const char* artist);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie paska postępu odtwarzania.                    // Draw the playback progress bar.              //
//                                                              //                                               //
// Parametry:                                                  // Parameters:                                   //
//                                                              //                                               //
// currentTime                                                  // currentTime                                   //
//      Aktualny czas odtwarzania.                             //      Current playback time.                   //
//                                                              //                                               //
// totalTime                                                    // totalTime                                     //
//      Całkowity czas utworu.                                 //      Total track duration.                     //
//                                                              //                                               //
// progress                                                     // progress                                      //
//      Postęp odtwarzania w procentach.                       //      Playback progress in percent.             //
//                                                              //                                               //
// playing                                                       // playing                                       //
//      true  - odtwarzanie                                    //      true  - playing                            //
//      false - pauza / zatrzymanie.                           //      false - paused / stopped.                 //
//----------------------------------------------------------//-----------------------------------------------//

    void drawPlaybackBar(
        const char* currentTime,
        const char* totalTime,
        int progress,
        bool playing);


    //----------------------------------------------------------//-----------------------------------------------//
    // Rysowanie aktualnego poziomu głośności.                 // Draw the current volume level.                //
//                                                              //                                               //
// volume                                                        // volume                                        //
//      Aktualny poziom głośności (0...100%).                  //      Current volume level (0...100%).         //
//----------------------------------------------------------//-----------------------------------------------//

    void drawVolume(int volume);



    //----------------------------------------------------------//-----------------------------------------------//
    // Stan przewijania tytułu.                                // Title scrolling state.                       //
    //----------------------------------------------------------//-----------------------------------------------//

    ScrollState titleScroll;


    //----------------------------------------------------------//-----------------------------------------------//
    // Stan przewijania wykonawcy.                             // Artist scrolling state.                     //
//----------------------------------------------------------//-----------------------------------------------//

    ScrollState artistScroll;


    //----------------------------------------------------------//-----------------------------------------------//
    // Stan ekranu bezczynności.                               // Idle screen state.                            //
//                                                              //                                               //
// false - wyświetlany jest ekran odtwarzacza                  // false - the player screen is displayed        //
// true  - wyświetlany jest ekran zegara                       // true  - the clock screen is displayed         //
//----------------------------------------------------------//-----------------------------------------------//

    bool idleScreenActive;


    //----------------------------------------------------------//-----------------------------------------------//
    // Czas ostatniej istotnej aktywności.                     // Time of the last significant activity.       //
//                                                              //                                               //
// Nie uwzględniamy tutaj zmian samego czasu odtwarzania.      // Changes to playback time alone are not        //
// Dzięki temu zegar może pojawić się również podczas          // considered. This allows the clock to appear   //
// ciągłego odtwarzania muzyki.                                // even while music is continuously playing.     //
//----------------------------------------------------------//-----------------------------------------------//

    uint32_t lastActivityMillis;

};


#endif