//==============================================================//-----------------------------------------------//
// Projekt : UP2Stream Display                              // Project : UP2Stream Display                   //
// Plik    : Up2StreamClient.h                              // File    : Up2StreamClient.h                   //
//                                                          //                                               //
// Opis:                                                    // Description:                                  //
//                                                          //                                               //
// Klasa odpowiedzialna za komunikację UART z modułem       // Class responsible for UART communication     //
// Arylic Up2Stream.                                        // with the Arylic Up2Stream module.            //
//                                                          //                                               //
// Zadania klasy:                                           // Class responsibilities:                     //
//                                                          //                                               //
// • odbiór danych z UART,                                  // • receiving data from UART,                  //
// • składanie komunikatów,                                 // • assembling messages,                       //
// • analiza protokołu,                                     // • parsing the protocol,                      //
// • aktualizacja struktury PlayerState,                    // • updating the PlayerState structure,        //
// • zgłaszanie zmian do modułu Display.                    // • reporting changes to the Display module.   //
//                                                          //                                               //
// Klasa nie rysuje niczego na ekranie.                     // The class does not draw anything on the      //
//                                                          // display.                                      //
//==============================================================//-----------------------------------------------//


#ifndef UP2STREAMCLIENT_H
#define UP2STREAMCLIENT_H


//--------------------------------------------------------------//-----------------------------------------------//
// Biblioteki.                                                // Libraries.                                    //
//--------------------------------------------------------------//-----------------------------------------------//

#include <Arduino.h>

#include "PlayerState.h"

#include "ChangeFlags.h"


//==============================================================//-----------------------------------------------//
// Czas odebrany z modułu Up2Stream.                          // Time received from the Up2Stream module.     //
//                                                              //                                               //
// Dane pochodzą z odpowiedzi:                                // Data comes from the following response:      //
//                                                              //                                               //
// TME:YYYY-MM-DD HH:MM:SS (+offset);                         // TME:YYYY-MM-DD HH:MM:SS (+offset);           //
//==============================================================//-----------------------------------------------//

struct Up2StreamTime
{
    int year = 0;
    int month = 0;
    int day = 0;

    int hour = 0;
    int minute = 0;
    int second = 0;

    int utcOffset = 0;

    bool valid = false;
};


//==============================================================//-----------------------------------------------//
// Klasa Up2StreamClient                                     // Up2StreamClient class                        //
//==============================================================//-----------------------------------------------//

class Up2StreamClient
{
public:

    //----------------------------------------------------------//-----------------------------------------------//
    // Konstruktor.                                            // Constructor.                                 //
    //----------------------------------------------------------//-----------------------------------------------//

    Up2StreamClient();


    //----------------------------------------------------------//-----------------------------------------------//
    // Inicjalizacja komunikacji UART.                         // Initialize UART communication.               //
    //----------------------------------------------------------//-----------------------------------------------//

    void begin(HardwareSerial& serial);


    //----------------------------------------------------------//-----------------------------------------------//
    // Wysłanie zapytania do modułu Up2Stream.                 // Send a query to the Up2Stream module.        //
    //                                                          //                                               //
    // Zapytanie powinno być zakończone znakiem ';'.            // The query should be terminated with ';'.    //
    //                                                          //                                               //
    // Przykłady:                                              // Examples:                                     //
    //     query("TME;");                                      //     query("TME;");                           //
    //     query("SRC;");                                      //     query("SRC;");                           //
    //     query("STA;");                                      //     query("STA;");                           //
    //----------------------------------------------------------//-----------------------------------------------//

    void query(const char* command);


    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualizacja.                                           // Update.                                      //
    //                                                          //                                               //
    // Funkcja odczytuje dane z UART i zwraca informacje       // The function reads data from UART and        //
    // o zmianach wykrytych w stanie odtwarzacza.              // returns information about changes detected   //
    //                                                          // in the player state.                         //
    //----------------------------------------------------------//-----------------------------------------------//

    ChangeFlags update(PlayerState& player);


    //----------------------------------------------------------//-----------------------------------------------//
    // Zwraca ostatni poprawnie odebrany czas TME.             // Return the last correctly received TME time. //
    //----------------------------------------------------------//-----------------------------------------------//

    const Up2StreamTime& getTime() const;


private:

    //----------------------------------------------------------//-----------------------------------------------//
    // Wskaźnik do portu UART.                                 // Pointer to the UART port.                    //
    //----------------------------------------------------------//-----------------------------------------------//

    HardwareSerial* uart;


    //----------------------------------------------------------//-----------------------------------------------//
    // Bufor odbierający pojedynczy komunikat UART.            // Buffer used to receive a single UART message.//
    //                                                          //                                               //
    // Przykład zawartości:                                    // Example contents:                            //
    //                                                          //                                               //
    // PLA:1;                                                  // PLA:1;                                        //
    // VOL:35;                                                 // VOL:35;                                       //
    // TIT:Hurt;                                               // TIT:Hurt;                                     //
    //----------------------------------------------------------//-----------------------------------------------//

    static const uint16_t RX_BUFFER_SIZE = 256;

    char rxBuffer[RX_BUFFER_SIZE];


    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualna pozycja zapisu w buforze.                      // Current write position in the buffer.        //
    //----------------------------------------------------------//-----------------------------------------------//

    uint16_t rxPosition;


    //----------------------------------------------------------//-----------------------------------------------//
    // Informacja o zmianie utworu.                            // Information about a track change.            //
    //                                                          //                                               //
    // Po odebraniu TIT poprzednia wartość totalTime            // After receiving TIT, the previous totalTime  //
    // przestaje być wiarygodna dla nowego utworu.             // value is no longer reliable for the new      //
    //                                                          // track.                                        //
    // Czekamy wtedy na pierwszą komendę ELP,                  // The client then waits for the first ELP      //
    // która poda aktualny czas oraz całkowity czas            // command, which provides the current position //
// nowego utworu.                                            // and total duration of the new track.          //
    //----------------------------------------------------------//-----------------------------------------------//

    bool waitingForTrackELP;


    //----------------------------------------------------------//-----------------------------------------------//
    // Analiza pojedynczego komunikatu odebranego z UART.      // Parse a single message received from UART.  //
    //                                                          //                                               //
    // Parametry:                                              // Parameters:                                   //
    //   message - kompletny komunikat zakończony ';'           //   message - complete message terminated      //
    //             // by ';'                                   //
    //   player  - aktualny stan odtwarzacza                   //   player  - current player state              //
    //                                                          //                                               //
    // Zwraca:                                                 // Returns:                                      //
    //   ChangeFlags informujące, które elementy interfejsu    //   ChangeFlags indicating which interface      //
    //   wymagają odświeżenia.                                 //   elements require an update.                //
    //----------------------------------------------------------//-----------------------------------------------//

    ChangeFlags processMessage(
        const char* message,
        PlayerState& player);


    //----------------------------------------------------------//-----------------------------------------------//
    // Ostatni poprawnie odebrany czas TME.                    // Last correctly received TME time.            //
    //----------------------------------------------------------//-----------------------------------------------//

    Up2StreamTime up2streamTime;
};


#endif