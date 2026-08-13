//==============================================================//-----------------------------------------------//
// Projekt : UP2Stream Display                              // Project : UP2Stream Display                   //
// Plik    : PlayerState.h                                  // File    : PlayerState.h                       //
//                                                          //                                               //
// Opis:                                                    // Description:                                  //
//                                                          //                                               //
// Definicja struktury PlayerState.                         // Definition of the PlayerState structure.     //
//                                                          //                                               //
// Struktura przechowuje kompletny stan odtwarzacza         // The structure stores the complete player      //
// przekazywany do modułu odpowiedzialnego za rysowanie     // state passed to the module responsible for    //
// interfejsu użytkownika.                                  // drawing the user interface.                   //
//                                                          //                                               //
// Dzięki wydzieleniu jej do osobnego pliku może być        // By keeping it in a separate file, it can be   //
// wykorzystywana przez wiele modułów programu:             // used by multiple program modules:             //
//                                                          //                                               //
// • komunikację z modułem Arylic,                          // • communication with the Arylic module,      //
// • moduł wyświetlacza,                                    // • display module,                             //
// • serwer WWW,                                            // • web server,                                 //
// • MQTT,                                                  // • MQTT,                                       //
// • zapis ustawień.                                        // • settings storage.                           //
//                                                          //                                               //
// Dzięki temu wszystkie moduły korzystają z jednego,      // This allows all modules to use one common     //
// wspólnego opisu aktualnego stanu odtwarzacza.            // description of the current player state.     //
//==============================================================//-----------------------------------------------//


#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H


//--------------------------------------------------------------//-----------------------------------------------//
// Biblioteka Arduino zapewniająca m.in. klasę String.        // Arduino library providing, among other       //
//                                                          // things, the String class.                     //
//--------------------------------------------------------------//-----------------------------------------------//

#include <Arduino.h>


//==============================================================//-----------------------------------------------//
// Struktura PlayerState                                     // PlayerState structure                         //
//                                                          //                                               //
// Zawiera komplet informacji opisujących aktualny stan      // Contains information describing the current  //
// odtwarzacza.                                              // player state.                                //
//                                                          //                                               //
// Struktura może być w przyszłości rozszerzana o kolejne    // The structure can be extended in the future  //
// informacje wymagane przez interfejs użytkownika lub        // with additional information required by the   //
// inne moduły programu.                                     // user interface or other program modules.      //
//==============================================================//-----------------------------------------------//

struct PlayerState
{

    //==========================================================//-----------------------------------------------//
    // Konstruktor domyślny                                    // Default constructor                         //
    //                                                          //                                               //
    // Inicjalizuje pola struktury wartościami domyślnymi.     // Initializes the structure fields with        //
    //                                                          // default values.                               //
    //                                                          //                                               //
    // Dzięki temu obiekt PlayerState może być bezpiecznie     // This allows a PlayerState object to be safely //
// wykorzystywany do porównywania oraz aktualizacji interfejsu // used for state comparison and user interface //
// użytkownika.                                               // updates.                                      //
    //==========================================================//-----------------------------------------------//

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
        playlistIndex = 0;

        playlistCount = 0;

        muted = false;

        audioActive = false;
    }


    //==========================================================//-----------------------------------------------//
    // Źródło odtwarzania                                      // Playback source                              //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualnie wybrane źródło dźwięku.                       // Currently selected audio source.             //
    //                                                          //                                               //
    // Przykłady:                                               // Examples:                                     //
    //                                                          //                                               //
    // NET                                                       // NET                                           //
    // LINE-IN                                                   // LINE-IN                                       //
    // USB                                                       // USB                                           //
    // BT                                                        // BT                                            //
    //----------------------------------------------------------//-----------------------------------------------//

    String source;


    //----------------------------------------------------------//-----------------------------------------------//
    // Nazwa wykonawcy.                                        // Artist name.                                  //
    //----------------------------------------------------------//-----------------------------------------------//

    String artist;


    //----------------------------------------------------------//-----------------------------------------------//
    // Tytuł aktualnie odtwarzanego utworu.                   // Title of the currently playing track.         //
    //----------------------------------------------------------//-----------------------------------------------//

    String title;


    //==========================================================//-----------------------------------------------//
    // Czas odtwarzania                                        // Playback time                                //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualny czas odtwarzania.                              // Current playback time.                       //
    //                                                          //                                               //
    // Przykład:                                                // Example:                                      //
    // 02:15                                                     // 02:15                                         //
    //----------------------------------------------------------//-----------------------------------------------//

    const char* currentTime;


    //----------------------------------------------------------//-----------------------------------------------//
    // Całkowity czas trwania utworu.                          // Total track duration.                        //
    //                                                          //                                               //
    // Przykład:                                                // Example:                                      //
    // 08:26                                                     // 08:26                                         //
    //----------------------------------------------------------//-----------------------------------------------//

    const char* totalTime;


    //==========================================================//-----------------------------------------------//
    // Postęp odtwarzania                                      // Playback progress                            //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Procentowy postęp odtwarzania.                          // Playback progress in percent.                //
    //                                                          //                                               //
    // Zakres:                                                  // Range:                                        //
    // 0...100                                                  // 0...100                                        //
    //----------------------------------------------------------//-----------------------------------------------//

    int progress;


    //==========================================================//-----------------------------------------------//
    // Głośność                                                 // Volume                                        //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualny poziom głośności.                              // Current volume level.                        //
    //                                                          //                                               //
    // Zakres:                                                  // Range:                                        //
    // 0...100                                                  // 0...100                                        //
    //----------------------------------------------------------//-----------------------------------------------//

    int volume;


    //==========================================================//-----------------------------------------------//
    // Informacje rozszerzone                                  // Extended information                          //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Nazwa albumu.                                            // Album name.                                   //
    //                                                          //                                               //
    // Przesyłana przez moduł Up2Stream komunikatem:            // Sent by the Up2Stream module using:           //
    //                                                          //                                               //
    // ALB:...                                                  // ALB:...                                        //
    //----------------------------------------------------------//-----------------------------------------------//

    String album;


    //----------------------------------------------------------//-----------------------------------------------//
    // Nazwa dostawcy usługi.                                  // Service provider name.                        //
    //                                                          //                                               //
    // Przykłady:                                               // Examples:                                     //
    //                                                          //                                               //
    // Spotify                                                  // Spotify                                       //
    // Tidal                                                    // Tidal                                         //
    // Radio                                                    // Radio                                         //
    // unknown                                                  // unknown                                       //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // VND:...                                                  // VND:...                                        //
    //----------------------------------------------------------//-----------------------------------------------//

    String vendor;


    //==========================================================//-----------------------------------------------//
    // Dane źródłowe odtwarzania                               // Playback source data                         //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualna pozycja odtwarzania.                           // Current playback position.                   //
    //                                                          //                                               //
    // Jednostka:                                               // Unit:                                         //
    //                                                          //                                               //
    // milisekundy                                              // milliseconds                                  //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // ELP:12345/45678;                                         // ELP:12345/45678;                              //
    //----------------------------------------------------------//-----------------------------------------------//

    uint32_t elapsedMs;


    //----------------------------------------------------------//-----------------------------------------------//
    // Moment rozpoczęcia lokalnego odmierzania czasu.          // Start time of local playback timing.         //
    //                                                          //                                               //
    // Wartość pochodzi z millis() i służy do wyliczania       // The value comes from millis() and is used to  //
    // aktualnej pozycji odtwarzania pomiędzy komunikatami ELP. // calculate the current playback position       //
    //                                                          // between ELP messages.                         //
    //----------------------------------------------------------//-----------------------------------------------//

    uint32_t playbackStartMillis;


    //----------------------------------------------------------//-----------------------------------------------//
    // Całkowity czas utworu.                                  // Total track duration.                        //
    //                                                          //                                               //
    // Jednostka:                                               // Unit:                                         //
    //                                                          //                                               //
    // milisekundy                                              // milliseconds                                  //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // ELP:12345/45678;                                         // ELP:12345/45678;                              //
    //----------------------------------------------------------//-----------------------------------------------//

    uint32_t totalMs;


    //==========================================================//-----------------------------------------------//
    // Informacje o playliście                                 // Playlist information                         //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Numer aktualnego utworu.                               // Index of the current track.                  //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // PLI:7/20;                                                // PLI:7/20;                                     //
    //----------------------------------------------------------//-----------------------------------------------//

    uint16_t playlistIndex;


    //----------------------------------------------------------//-----------------------------------------------//
    // Liczba utworów.                                         // Number of tracks.                            //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // PLI:7/20;                                                // PLI:7/20;                                     //
    //----------------------------------------------------------//-----------------------------------------------//

    uint16_t playlistCount;


    //==========================================================//-----------------------------------------------//
    // Dodatkowe stany odtwarzacza                             // Additional player states                    //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Wyciszenie.                                             // Mute state.                                  //
    //                                                          //                                               //
    // true  - Mute                                             // true  - muted                                //
    // false - normalne odtwarzanie                             // false - normal playback                      //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // MUT:0                                                    // MUT:0                                         //
    // MUT:1                                                    // MUT:1                                         //
    //----------------------------------------------------------//-----------------------------------------------//

    bool muted;


    //----------------------------------------------------------//-----------------------------------------------//
    // Informacja o aktywnym strumieniu audio.                 // Information about an active audio stream.   //
    //                                                          //                                               //
    // false - brak strumienia                                 // false - no active stream                    //
    // true  - audio aktywne                                   // true  - audio active                        //
    //                                                          //                                               //
    // Komunikat:                                               // Message:                                      //
    //                                                          //                                               //
    // AUD:0                                                    // AUD:0                                         //
    // AUD:1                                                    // AUD:1                                         //
    //----------------------------------------------------------//-----------------------------------------------//

    bool audioActive;


    //==========================================================//-----------------------------------------------//
    // Stan zasilania / pracy modułu Up2Stream                 // Up2Stream power / operating state            //
    //==========================================================//-----------------------------------------------//
    //
    // false - urządzenie pracuje normalnie                    // false - device operates normally             //
    // true  - urządzenie znajduje się w trybie standby        // true  - device is in standby mode             //
    //                                                          //                                               //
    // Stan ten jest niezależny od:                            // This state is independent of:                 //
    // - playing,                                               // - playing,                                    //
    // - mute,                                                  // - mute,                                       //
    // - ekranu zegara.                                         // - clock screen.                               //
    //==========================================================//-----------------------------------------------//

    bool standby = false;


    //==========================================================//-----------------------------------------------//
    // Stan odtwarzacza                                        // Player state                                 //
    //==========================================================//-----------------------------------------------//

    //----------------------------------------------------------//-----------------------------------------------//
    // Informacja o stanie odtwarzania.                        // Playback state information.                   //
    //                                                          //                                               //
    // true  - odtwarzanie                                     // true  - playing                               //
    // false - pauza                                           // false - paused                                //
    //----------------------------------------------------------//-----------------------------------------------//

    bool playing;
};


#endif