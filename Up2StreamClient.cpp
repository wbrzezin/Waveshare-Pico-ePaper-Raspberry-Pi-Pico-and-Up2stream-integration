//======================================================//-----------------------------------------------//
// Projekt : UP2Stream Display                             // Project : UP2Stream Display                 //
// Plik    : Up2StreamClient.cpp                           // File    : Up2StreamClient.cpp               //
//                                                         //                                             //
// Opis:                                                   // Description:                                //
//                                                         //                                             //
// Implementacja klasy odpowiedzialnej za komunikację UART   // Implementation of the class responsible for UART communication //
// z modułem Arylic Up2Stream.                             // with the Arylic Up2Stream module.           //
//                                                         //                                             //
// Implementacja obejmuje:                                 // The implementation includes:                //
//                                                         //                                             //
// • konstruktor,                                          // • constructor,                              //
// • inicjalizację UART,                                   // • UART initialization,                      //
// • odbiór komunikatów UART,                              // • receiving UART messages,                  //
// • parser PLA / VOL / SRC / TIT / ELP / MUT / SYS / VND / TME, // • PLA / VOL / SRC / TIT / ELP / MUT / SYS / VND / TME parser, //
// • lokalne odmierzanie czasu odtwarzania,                // • local playback time tracking,             //
// • obsługę sklejonych komunikatów TIT + ELP.             // • handling of concatenated TIT + ELP messages. //
//======================================================//-----------------------------------------------//

#include "Up2StreamClient.h"


//======================================================//-----------------------------------------------//
// Konstruktor.                                            // Constructor.                                //
//                                                         //                                             //
// Zeruje wskaźnik do portu UART.                          // Clears the UART port pointer.               //
//======================================================//-----------------------------------------------//

Up2StreamClient::Up2StreamClient()
{
    //------------------------------------------------------//-----------------------------------------------//
    // Port UART nie został jeszcze przypisany.                // The UART port has not been assigned yet.    //
    //------------------------------------------------------//-----------------------------------------------//

    uart = nullptr;

    //------------------------------------------------------//-----------------------------------------------//
    // Początek bufora odbiorczego.                            // Start of the receive buffer.                //
    //------------------------------------------------------//-----------------------------------------------//

    rxPosition = 0;

    //------------------------------------------------------//-----------------------------------------------//
    // Na początku nie oczekujemy na ELP nowego utworu.        // Initially, we are not waiting for an ELP message for a new track. //
    //------------------------------------------------------//-----------------------------------------------//

    waitingForTrackELP = false;

    rxBuffer[0] = '\0';
}

//======================================================//-----------------------------------------------//
// Inicjalizacja.                                          // Initialization.                             //
//                                                         //                                             //
// Zapamiętuje port UART wykorzystywany do komunikacji     // Stores the UART port used for communication //
// z modułem Up2Stream.                                    // with the Up2Stream module.                  //
//                                                         //                                             //
// Sama konfiguracja prędkości transmisji będzie wykonywana   // The transmission speed configuration will be performed //
// w programie głównym.                                    // in the main program.                        //
//======================================================//-----------------------------------------------//

void Up2StreamClient::begin(HardwareSerial& serial)
{
    uart = &serial;
}

//======================================================//-----------------------------------------------//
// Wysłanie zapytania do modułu Up2Stream.                 // Send a query to the Up2Stream module.       //
//======================================================//-----------------------------------------------//
//                                                         //                                             //
// Funkcja nie oczekuje na odpowiedź.                      // The function does not wait for a response.  //
//                                                         //                                             //
// Wysyła jedynie komendę do UART. Odpowiedź zostanie odebrana   // It only sends the command to UART. The response will be received //
// podczas kolejnego wywołania update() i przekazana do istniejącego   // during the next call to update() and passed to the existing //
// parsera.                                                // parser.                                     //
//                                                         //                                             //
// Przykład:                                               // Example:                                    //
//                                                         //                                             //
//     query("TME;");                                      //     query("TME;");                          //
//                                                         //                                             //
// wysyła:                                                 // sends:                                      //
//                                                         //                                             //
//     TME;                                                //     TME;                                    //
//                                                         //                                             //
//======================================================//-----------------------------------------------//

void Up2StreamClient::query(const char* command)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Jeżeli UART nie został zainicjalizowany,                // If the UART has not been initialized,       //
//    // nie wykonujemy żadnej operacji.                   //    // no operation is performed.            //
//------------------------------------------------------//-----------------------------------------------//

    if (uart == nullptr)
        return;

    

    //------------------------------------------------------//-----------------------------------------------//
    // Wyślij zapytanie.                                       // Send the query.                             //
//------------------------------------------------------//-----------------------------------------------//

    uart->print(command);
}

const Up2StreamTime& Up2StreamClient::getTime() const
{
    return up2streamTime;
}

//======================================================//-----------------------------------------------//
// Aktualizacja.                                          // Update.                                       //
//                                                        //                                               //
// Odczytuje wszystkie oczekujące znaki UART,             // Reads all pending UART characters,             //
// składa kompletne komunikaty i przekazuje je            // assembles complete messages and passes them    //
// do parsera.                                            // to the parser.                                 //
//                                                        //                                               //
// W jednym przebiegu może odebrać wiele komunikatów,     // A single pass may receive multiple messages,   //
// zaktualizować PlayerState i zwrócić połączone          // update PlayerState and return the combined     //
// ChangeFlags.                                           // ChangeFlags.                                  //
//======================================================//-----------------------------------------------//

ChangeFlags Up2StreamClient::update(PlayerState& player)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Jeżeli UART nie został jeszcze zainicjalizowany,        // If the UART has not been initialized yet,   //
    // nie wykonujemy żadnych działań.                         // no action is performed.                     //
    //------------------------------------------------------//-----------------------------------------------//

    if (uart == nullptr)
    {
        return ChangeFlags::None;
    }



    //------------------------------------------------------//-----------------------------------------------//
    // Zestaw zmian wykrytych podczas tego wywołania update().   // Set of changes detected during this update() call. //
    //                                                         //                                             //
    // Nie zwracamy już natychmiast po odebraniu komunikatu.   // We no longer return immediately after receiving a message. //
    // Dzięki temu w jednym przebiegu możemy:                  // This allows us to, during one pass:         //
    //                                                         //                                             //
    // • odebrać komunikat UART,                               // • receive a UART message,                   //
    // • zaktualizować PlayerState,                            // • update PlayerState,                       //
    // • zaktualizować lokalny zegar,                          // • update the local clock,                   //
    // • zwrócić kompletny zestaw zmian.                       // • return the complete set of changes.       //
    //------------------------------------------------------//-----------------------------------------------//

    ChangeFlags changes = ChangeFlags::None;

    //------------------------------------------------------//-----------------------------------------------//
    // Odczytaj wszystkie dostępne znaki z UART.               // Read all available characters from UART.    //
    //------------------------------------------------------//-----------------------------------------------//

    while (uart->available())
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Pobierz jeden znak.                                     // Read one character.                         //
        //------------------------------------------------------//-----------------------------------------------//

        char c = uart->read();

        //------------------------------------------------------//-----------------------------------------------//
        // Zabezpieczenie przed przepełnieniem bufora.             // Prevent buffer overflow.                    //
        //------------------------------------------------------//-----------------------------------------------//

        if (rxPosition >= RX_BUFFER_SIZE - 1)
        {
            rxPosition = 0;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Dopisz znak do bufora.                                  // Append the character to the buffer.         //
        //------------------------------------------------------//-----------------------------------------------//

        rxBuffer[rxPosition++] = c;

        //------------------------------------------------------//-----------------------------------------------//
        // Zakończenie napisu.                                     // Terminate the string.                       //
        //------------------------------------------------------//-----------------------------------------------//

        rxBuffer[rxPosition] = '\0';


        //------------------------------------------------------//-----------------------------------------------//
        // Czy odebrano kompletny komunikat?                       // Was a complete message received?            //
        //------------------------------------------------------//-----------------------------------------------//

        if (c == ';')
        {
            //------------------------------------------------------//-----------------------------------------------//
            // Przetwórz komunikat.                                    // Process the message.                        //
            //------------------------------------------------------//-----------------------------------------------//


            
            ChangeFlags messageChanges =
                processMessage(
                    rxBuffer,
                    player);

            //------------------------------------------------------//-----------------------------------------------//
            // Dodaj wykryte zmiany do wspólnego zestawu.              // Add the detected changes to the combined set. //
            //------------------------------------------------------//-----------------------------------------------//

            changes =
                changes | messageChanges;



            //------------------------------------------------------//-----------------------------------------------//
            // Wyczyść bufor.                                          // Clear the buffer.                           //
            //------------------------------------------------------//-----------------------------------------------//

            rxPosition = 0;

            rxBuffer[0] = '\0';
        }
    }

    //------------------------------------------------------//-----------------------------------------------//
    // Lokalny zegar odtwarzania.                              // Local playback clock.                       //
    //                                                         //                                             //
    // ELP synchronizuje pozycję.                              // ELP synchronizes the playback position.     //
    // Pomiędzy komunikatami ELP czas odmierzamy lokalnie.     // Between ELP messages, playback time is tracked locally. //
    //------------------------------------------------------//-----------------------------------------------//

    static int lastDisplayedSecond = -1;

    if (player.playing &&
        player.totalMs > 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Aktualny czas systemowy.                                // Current system time.                        //
        //------------------------------------------------------//-----------------------------------------------//

        uint32_t now = millis();

     //------------------------------------------------------//-----------------------------------------------//
     // Czas, który upłynął od ostatniego punktu odniesienia.   // Time elapsed since the last reference point. //
     //------------------------------------------------------//-----------------------------------------------//
   

        uint32_t delta =
           now - player.playbackStartMillis;

    //------------------------------------------------------//-----------------------------------------------//
    // Aktualna pozycja utworu.                                // Current track position.                     //
    //------------------------------------------------------//-----------------------------------------------//

        uint32_t localElapsed =
                 player.elapsedMs + delta;

    //------------------------------------------------------//-----------------------------------------------//
    // Nowy punkt odniesienia.                                 // New reference point.                        //
    //------------------------------------------------------//-----------------------------------------------//


        player.playbackStartMillis = now;


        //------------------------------------------------------//-----------------------------------------------//
        // Nie przekraczaj końca utworu.                           // Do not exceed the end of the track.         //
        //------------------------------------------------------//-----------------------------------------------//

        if (localElapsed > player.totalMs)
        {
            localElapsed = player.totalMs;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Sekunda przed formatowaniem MM:SS.                      // Seconds before formatting as MM:SS.         //
        //                                                         //                                             //
        // UWAGA:                                                  // NOTE:                                       //
        // przechowujemy całkowitą liczbę sekund,                  // the total number of seconds is stored,      //
        // np. 135 dla 02:15.                                      // e.g. 135 for 02:15.                         //
        //------------------------------------------------------//-----------------------------------------------//

        uint32_t totalElapsedSeconds =
            localElapsed / 1000;

        //------------------------------------------------------//-----------------------------------------------//
        // Sprawdź, czy zmieniła się wyświetlana sekunda.          // Check whether the displayed second has changed. //
        //------------------------------------------------------//-----------------------------------------------//

        bool secondChanged =
            ((int)totalElapsedSeconds !=
             lastDisplayedSecond);

        //------------------------------------------------------//-----------------------------------------------//
        // Aktualizacja czasu MM:SS.                               // Update the MM:SS time.                      //
        //------------------------------------------------------//-----------------------------------------------//

        uint32_t displaySeconds =
            totalElapsedSeconds;

        uint32_t displayMinutes =
            displaySeconds / 60;

        displaySeconds %= 60;

        //------------------------------------------------------//-----------------------------------------------//
        // Bufor aktualnego czasu.                                 // Current time buffer.                        //
        //------------------------------------------------------//-----------------------------------------------//

        static char currentTimeBuffer[12];

        snprintf(
            currentTimeBuffer,
            sizeof(currentTimeBuffer),
            "%02lu:%02lu",
            (unsigned long)displayMinutes,
            (unsigned long)displaySeconds);

        //------------------------------------------------------//-----------------------------------------------//
        // Zapisz aktualną pozycję.                                // Store the current position.                 //
        //------------------------------------------------------//-----------------------------------------------//

        player.elapsedMs = localElapsed;

        player.currentTime =
            currentTimeBuffer;

        //------------------------------------------------------//-----------------------------------------------//
        // Aktualizacja postępu.                                   // Update playback progress.                   //
        //------------------------------------------------------//-----------------------------------------------//

        player.progress =
            (int)(
                (player.elapsedMs * 100UL) /
                player.totalMs);

        //------------------------------------------------------//-----------------------------------------------//
        // Jeżeli zmieniła się sekunda,                            // If the second has changed,                  //
        // zgłoś zmianę do Display.                                // report the change to Display.               //
        //------------------------------------------------------//-----------------------------------------------//

        if (secondChanged)
        {
            lastDisplayedSecond =
                (int)totalElapsedSeconds;

            changes =
                changes |
                ChangeFlags::CurrentTime |
                ChangeFlags::Progress;
        }
    }

    //------------------------------------------------------//-----------------------------------------------//
    // Zwróć wszystkie zmiany wykryte podczas tego przebiegu.   // Return all changes detected during this pass. //
    //------------------------------------------------------//-----------------------------------------------//

    return changes;
}
//======================================================//-----------------------------------------------//
// Analiza pojedynczego komunikatu.                     // Parse a single message.                         //
//                                                        //                                               //
// Obsługiwane komendy:                                  // Supported commands:                            //
//                                                        //                                               //
// PLA:0; / PLA:1;                                       // PLA:0; / PLA:1;                                //
// VOL:0; ... VOL:100;                                   // VOL:0; ... VOL:100;                            //
// SYS:ON; / SYS:STANDBY;                                // SYS:ON; / SYS:STANDBY;                         //
// SRC:...; / VND:...;                                  // SRC:...; / VND:...;                            //
// TIT:...; / ART:...;                                  // TIT:...; / ART:...;                            //
// ELP:elapsed/total; / MUT:0; / MUT:1;                 // ELP:elapsed/total; / MUT:0; / MUT:1;          //
// TME:YYYY-MM-DD HH:MM:SS (+offset);                   // TME:YYYY-MM-DD HH:MM:SS (+offset);              //
//                                                        //                                               //
// Funkcja aktualizuje PlayerState i zwraca flagi,    // The function updates PlayerState and returns   //
// które informują Display o zmianach wymagających    // flags indicating which display elements require //
// odświeżenia.                                          // an update.                                     //
//======================================================//-----------------------------------------------//

ChangeFlags Up2StreamClient::processMessage(
    const char* message,
    PlayerState& player)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Pomijanie znaków CR i LF znajdujących się na początku   // Skip CR and LF characters at the beginning of the //
    // komunikatu.                                             // message.                                    //
    //                                                         //                                             //
    // Up2Stream może wysyłać komunikaty poprzedzone znakami:   // Up2Stream may send messages prefixed with:  //
    //                                                         //                                             //
    // CR = '\r'                                               // CR = '\r'                                   //
    // LF = '\n'                                               // LF = '\n'                                   //
    //                                                         //                                             //
    // Dzięki temu parser może poprawnie rozpoznać np.:        // This allows the parser to correctly recognize, for example: //
    //                                                         //                                             //
    // \rVOL:80;                                               // \rVOL:80;                                   //
    //                                                         //                                             //
    // jako:                                                   // as:                                         //
    //                                                         //                                             //
    // VOL:80;                                                 // VOL:80;                                     //
    //------------------------------------------------------//-----------------------------------------------//

    while (*message == '\r' ||
           *message == '\n')
    {
        message++;
    }

        //======================================================//-----------------------------------------------//
    // Komenda PLAY / PAUSE                                    // PLAY / PAUSE command                        //
    //======================================================//-----------------------------------------------//

    //------------------------------------------------------//-----------------------------------------------//
    // Przykłady:                                              // Examples:                                    //
    //                                                         //                                             //
    // PLA:0;                                                  // PLA:0;                                      //
    // PLA:1;                                                  // PLA:1;                                      //
    //------------------------------------------------------//-----------------------------------------------//

        if (strncmp(message, "PLA:", 4) == 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Odczytaj stan odtwarzania.                              // Read the playback state.                    //
        //                                                         //                                             //
        // '0' = pauza                                             // '0' = pauza                                 //
        // '1' = odtwarzanie                                       // '1' = odtwarzanie                           //
        //------------------------------------------------------//-----------------------------------------------//

        bool newPlaying =
            (message[4] == '1');

        //------------------------------------------------------//-----------------------------------------------//
        // PAUSE                                                   // PAUSE                                       //
        //------------------------------------------------------//-----------------------------------------------//

        if (!newPlaying)
        {
            //------------------------------------------------------//-----------------------------------------------//
            // Jeżeli właśnie przechodzimy z PLAY do PAUSE,            // If we are switching from PLAY to PAUSE,     //
            // zapisz aktualną pozycję odtwarzania.                    // store the current playback position.        //
            //                                                         //                                             //
            // Dzięki temu po wznowieniu nie zaczniemy                 // This prevents playback from starting again  //
            // odmierzać czasu od poprzedniego punktu                  // from the previous ELP synchronization       //
            // synchronizacji ELP.                                     // point after resuming.                       //
            //------------------------------------------------------//-----------------------------------------------//

            if (player.playing &&
                player.totalMs > 0)
            {
                uint32_t now = millis();

                uint32_t delta =
                    now - player.playbackStartMillis;

                uint32_t pausedElapsed =
                    player.elapsedMs + delta;

                //------------------------------------------------------//-----------------------------------------------//
                // Nie przekraczaj końca utworu.                           // Do not exceed the end of the track.         //
                //------------------------------------------------------//-----------------------------------------------//

                if (pausedElapsed > player.totalMs)
                {
                    pausedElapsed =
                        player.totalMs;
                }

                player.elapsedMs =
                    pausedElapsed;
            }

            //------------------------------------------------------//-----------------------------------------------//
            // Ustaw stan PAUSE.                                       // Set the PAUSE state.                        //
            //------------------------------------------------------//-----------------------------------------------//

            player.playing = false;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // PLAY                                                    // PLAY                                        //
        //------------------------------------------------------//-----------------------------------------------//

        else
        {
            //------------------------------------------------------//-----------------------------------------------//
            // Ustaw nowy punkt odniesienia dla lokalnego zegara.      // Set a new reference point for the local clock. //
            //                                                         //                                             //
            // Od tej chwili będziemy dodawać czas do aktualnego       // From this point on, elapsed time will be added to the current //
            // elapsedMs.                                              // elapsedMs.                                  //
            //------------------------------------------------------//-----------------------------------------------//

            player.playbackStartMillis =
                millis();

            //------------------------------------------------------//-----------------------------------------------//
            // Ustaw stan PLAY.                                        // Set the PLAY state.                         //
            //------------------------------------------------------//-----------------------------------------------//

            player.playing = true;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Poinformuj wyświetlacz o zmianie stanu odtwarzania.     // Notify the display about the playback state change. //
        //------------------------------------------------------//-----------------------------------------------//

        return ChangeFlags::PlayState;
    }


    //======================================================//-----------------------------------------------//
    // Komenda VOL - poziom głośności                          // VOL command - volume level                  //
    //======================================================//-----------------------------------------------//

    //------------------------------------------------------//-----------------------------------------------//
    // Przykład:                                               // Example:                                    //
    //                                                         //                                             //
    // VOL:87;                                                 // VOL:87;                                     //
    //------------------------------------------------------//-----------------------------------------------//

    if (strncmp(message, "VOL:", 4) == 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Odczytaj poziom głośności.                              // Read the volume level.                      //
        //------------------------------------------------------//-----------------------------------------------//

        player.volume = atoi(message + 4);

        //------------------------------------------------------//-----------------------------------------------//
        // Poinformuj wyświetlacz o zmianie poziomu głośności.     // Notify the display about the volume level change. //
        //------------------------------------------------------//-----------------------------------------------//

        return ChangeFlags::Volume;
    }

//======================================================//-----------------------------------------------//
// Komenda SYS - stan pracy modułu Up2Stream               // SYS command - Up2Stream operating state     //
//======================================================//-----------------------------------------------//
//                                                         //                                             //
// Przykłady:                                              // Examples:                                    //
//                                                         //                                             //
// SYS:ON;                                                 // SYS:ON;                                     //
// SYS:STANDBY;                                            // SYS:STANDBY;                                //
//                                                         //                                             //
// SYS:STANDBY oznacza przejście urządzenia w tryb         // SYS:STANDBY means that the device enters standby mode. //
// czuwania.                                               // czuwania.                                   //
//                                                         //                                             //
// SYS:ON oznacza powrót urządzenia do normalnej pracy.    // SYS:ON means that the device returns to normal operation. //
//                                                         //                                             //
// Stan SYS jest niezależny od PLA oraz MUT.               // The SYS state is independent of PLA and MUT.   //
//======================================================//-----------------------------------------------//

if (strncmp(message, "SYS:", 4) == 0)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Sprawdzenie stanu urządzenia.                           // Check the device state.                     //
    //------------------------------------------------------//-----------------------------------------------//

    if (strncmp(message + 4, "STANDBY", 7) == 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Up2Stream przeszedł w tryb standby.                     // Up2Stream has entered standby mode.         //
        //------------------------------------------------------//-----------------------------------------------//

        player.standby = true;
    }
    else if (strncmp(message + 4, "ON", 2) == 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Up2Stream powrócił do normalnej pracy.                  // Up2Stream has returned to normal operation. //
        //------------------------------------------------------//-----------------------------------------------//

        player.standby = false;
    }

    //------------------------------------------------------//-----------------------------------------------//
    // Poinformuj pozostałą część programu o zmianie stanu.    // Notify the rest of the program about the state change. //
    //------------------------------------------------------//-----------------------------------------------//

    return ChangeFlags::Standby;
}

//======================================================//-----------------------------------------------//
// Komenda SRC - źródło odtwarzania                        // SRC command - playback source               //
//======================================================//-----------------------------------------------//
//                                                         //                                             //
// Przykłady:                                              // Examples:                                    //
//                                                         //                                             //
// SRC:NET;                                                // SRC:NET;                                    //
// SRC:LINE-IN;                                            // SRC:LINE-IN;                                //
// SRC:USB;                                                // SRC:USB;                                    //
// SRC:BT;                                                 // SRC:BT;                                     //
//                                                         //                                             //
// Przy każdej zmianie źródła kasujemy poprzednią informację   // On every source change, clear the previous VND information. //
// VND. Nie wysyłamy zapytania VND; do Up2Stream.          // VND. We do not send a VND query to Up2Stream. //
//                                                         //                                             //
// Jeżeli Up2Stream następnie prześle:                     // If Up2Stream subsequently sends:          //
//                                                         //                                             //
// VND:Spotify;                                            // VND:Spotify;                                //
//                                                         //                                             //
// informacja zostanie zapisana osobno przez parser VND.   // the information is stored separately by the VND parser. //
//======================================================//-----------------------------------------------//

if (strncmp(message, "SRC:", 4) == 0)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Zapisz nazwę źródła.                                    // Store the source name.                      //
    //------------------------------------------------------//-----------------------------------------------//

    player.source =
        String(message + 4);


    //------------------------------------------------------//-----------------------------------------------//
    // Usuń końcowy znak ';'.                                  // Remove the trailing ';' character.          //
    //------------------------------------------------------//-----------------------------------------------//

    if (player.source.endsWith(";"))
    {
        player.source.remove(
            player.source.length() - 1);
    }


    //------------------------------------------------------//-----------------------------------------------//
    // Nowe źródło oznacza nowy kontekst VND.                  // A new source means a new VND context.       //
    //                                                         //                                             //
    // Nie wolno pozostawić informacji związanej z             // Information associated with the             //
    // poprzednim źródłem.                                     // previous source must not be retained.       //
    //------------------------------------------------------//-----------------------------------------------//

    player.vendor = "";


    //------------------------------------------------------//-----------------------------------------------//
    // Poinformuj wyświetlacz o zmianie źródła.                // Notify the display about the source change. //
    //------------------------------------------------------//-----------------------------------------------//

    return ChangeFlags::Source;
}


//======================================================//-----------------------------------------------//
// Komenda VND - dostawca / usługa źródła                  // VND command - source provider / service     //
//======================================================//-----------------------------------------------//
//                                                         //                                             //
// Przykład:                                               // Example:                                    //
//                                                         //                                             //
// VND:Spotify;                                            // VND:Spotify;                                //
//                                                         //                                             //
// Up2Stream może przesłać tę informację spontanicznie.    // Up2Stream may send this information spontaneously. //
// Nie wysyłamy do urządzenia zapytania VND;.              // We do not send a VND query to the device.   //
//                                                         //                                             //
// Informacja jest zapisywana w PlayerState::vendor.       // The information is stored in PlayerState::vendor. //
//======================================================//-----------------------------------------------//

if (strncmp(message, "VND:", 4) == 0)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Zapisz nazwę dostawcy/usługi.                           // Store the provider/service name.            //
    //------------------------------------------------------//-----------------------------------------------//

    player.vendor =
        String(message + 4);


    //------------------------------------------------------//-----------------------------------------------//
    // Usuń końcowy znak ';'.                                  // Remove the trailing ';' character.          //
    //------------------------------------------------------//-----------------------------------------------//

    if (player.vendor.endsWith(";"))
    {
        player.vendor.remove(
            player.vendor.length() - 1);
    }


    //------------------------------------------------------//-----------------------------------------------//
    // Poinformuj wyświetlacz o zmianie informacji VND.        // Notify the display about the VND information change. //
    //------------------------------------------------------//-----------------------------------------------//

    return ChangeFlags::Vendor;
}

//======================================================//-----------------------------------------------//
// Komenda TIT - tytuł utworu                              // TIT command - track title                   //
//======================================================//-----------------------------------------------//
//                                                         //                                             //
// Normalny komunikat:                                     // Normal message:                             //
//                                                         //                                             //
// TIT:Mandalay;                                           // TIT:Mandalay;                               //
//                                                         //                                             //
// Up2Stream może jednak skleić kolejne pola:              // However, Up2Stream may concatenate subsequent fields: //
//                                                         //                                             //
// TIT:Historia choALB:Gra?;                               // TIT:Historia choALB:Gra?;                   //
//                                                         //                                             //
// albo:                                                   // albo:                                       //
//                                                         //                                             //
// TIT:Historia choART:El Dupa;                            // TIT:Historia choART:El Dupa;                //
//                                                         //                                             //
// Dlatego szukamy początku następnego pola                // Therefore, we search for the beginning of the next field //
// i kończymy tytuł przed ART: lub ALB:.                   // and terminate the title before ART: or ALB:. //
//======================================================//-----------------------------------------------//

    if (strncmp(message, "TIT:", 4) == 0)
    {



        //------------------------------------------------------//-----------------------------------------------//
        // Początek właściwego tytułu.                             // Start of the actual title.                  //
        //------------------------------------------------------//-----------------------------------------------//

        const char* titleStart =
            message + 4;

        //------------------------------------------------------//-----------------------------------------------//
        // Rozpoczęcie obsługi nowego utworu.                      // Start processing a new track.               //
        //                                                         //                                             //
        // Sam komunikat TIT oznacza, że zmienił się utwór,        // The TIT message itself means that the track has changed, //
        // ale nie znamy jeszcze jego całkowitego czasu.           // but its total duration is not known yet.    //
        //                                                         //                                             //
        // Czas całkowity zostanie uzupełniony dopiero po          // The total duration will be filled in only after //
        // odebraniu pierwszego ELP dla nowego utworu.             // receiving the first ELP for the new track.  //
        //------------------------------------------------------//-----------------------------------------------//

        waitingForTrackELP = true;

        //------------------------------------------------------//-----------------------------------------------//
        // Zerujemy lokalny stan czasu.                            // Reset the local time state.                 //
        //                                                         //                                             //
        // Nie wolno pozostawić tutaj wartości poprzedniego        // The previous track value must not remain here, //
        // utworu, ponieważ przez pewien czas może ona być         // because it could otherwise remain visible for some time. //
        // jeszcze wyświetlana.                                    // on the display.                             //
        //------------------------------------------------------//-----------------------------------------------//

        player.elapsedMs = 0;

        player.currentTime =
            "00:00";

        player.totalTime =
            "--:--";

        player.progress =
            0;    

        //------------------------------------------------------//-----------------------------------------------//
        // Up2Stream może skleić TIT z kolejnym komunikatem ELP.   // Up2Stream may concatenate TIT with the following ELP message. //
        //                                                         //                                             //
        // Przykład:                                               // Example:                                    //
        //                                                         //                                             //
        // TIT:Ballada oELP:31880/137426;                          // TIT:Ballada oELP:31880/137426;              //
        //                                                         //                                             //
        // Tytuł kończy się więc przed "ELP:".                     // The title therefore ends before "ELP:".     //
        // Fragment ELP przekazujemy ponownie do parsera,          // The ELP fragment is passed back to the parser, //
        // aby jednocześnie zaktualizować czas odtwarzania.        // so that the playback time can be updated at the same time. //
        //------------------------------------------------------//-----------------------------------------------//

        const char* embeddedELP =
            strstr(titleStart, "ELP:");

        //------------------------------------------------------//-----------------------------------------------//
        // Zmiany wygenerowane przez osadzony komunikat ELP.       // Changes generated by the embedded ELP message. //
        //------------------------------------------------------//-----------------------------------------------//

        ChangeFlags embeddedChanges =
            ChangeFlags::None;

        if (embeddedELP != nullptr)
        {
           

            //------------------------------------------------------//-----------------------------------------------//
            // Przetwórz np.:                                          // Process, for example:                       //
            //                                                         //                                             //
            // ELP:31880/137426;                                       // ELP:31880/137426;                           //
            //------------------------------------------------------//-----------------------------------------------//

            embeddedChanges =
                processMessage(
                    embeddedELP,
                    player);
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Ustal początkowy koniec tytułu.                         // Determine the initial end of the title.     //
        //                                                         //                                             //
        // W normalnym komunikacie jest nim średnik.               // In a normal message, this is the semicolon. //
        //------------------------------------------------------//-----------------------------------------------//

        const char* titleEnd =
            strchr(titleStart, ';');

        //------------------------------------------------------//-----------------------------------------------//
        // Poszukaj kolejnych pól sklejonych z tytułem.            // Search for subsequent fields concatenated with the title. //
        //------------------------------------------------------//-----------------------------------------------//

        const char* artistStart =
            strstr(titleStart, "ART:");

        const char* albumStart =
            strstr(titleStart, "ALB:");

        //------------------------------------------------------//-----------------------------------------------//
        // ART: rozpoczyna kolejne pole.                           // ART: starts the next field.                 //
        //------------------------------------------------------//-----------------------------------------------//

        if (artistStart != nullptr &&
            (titleEnd == nullptr ||
             artistStart < titleEnd))
        {
            titleEnd =
                artistStart;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // ALB: rozpoczyna kolejne pole.                           // ALB: starts the next field.                 //
        //------------------------------------------------------//-----------------------------------------------//

        if (albumStart != nullptr &&
            (titleEnd == nullptr ||
             albumStart < titleEnd))
        {
            titleEnd =
                albumStart;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // ELP: rozpoczyna sklejony komunikat pozycji.             // ELP: starts a concatenated playback-position message. //
        //                                                         //                                             //
        // Jest to istotne dla przypadku:                          // This is important for the following case:   //
        //                                                         //                                             //
        // TIT:Ballada oELP:31880/137426;                          // TIT:Ballada oELP:31880/137426;              //
        //                                                         //                                             //
        // Bez tego fragment "ELP:..." zostałby dopisany           // Without this, the "ELP:..." fragment would be appended //
        // do tytułu.                                              // to the title.                               //
        //------------------------------------------------------//-----------------------------------------------//

        if (embeddedELP != nullptr &&
            (titleEnd == nullptr ||
             embeddedELP < titleEnd))
        {
            titleEnd =
                embeddedELP;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Jeżeli znaleziono granicę tytułu, skopiuj tylko         // If a title boundary was found, copy only the //
        // właściwy fragment.                                      // actual title fragment.                      //
        //------------------------------------------------------//-----------------------------------------------//

        if (titleEnd != nullptr)
        {
            //------------------------------------------------------//-----------------------------------------------//
            // Oblicz długość właściwego tytułu.                       // Calculate the length of the actual title.   //
            //------------------------------------------------------//-----------------------------------------------//

            size_t titleLength =
                titleEnd - titleStart;

            //------------------------------------------------------//-----------------------------------------------//
            // Tymczasowy bufor tytułu.                                // Temporary title buffer.                     //
            //------------------------------------------------------//-----------------------------------------------//

            char titleBuffer[RX_BUFFER_SIZE];

            //------------------------------------------------------//-----------------------------------------------//
            // Zabezpieczenie przed przepełnieniem.                    // Prevent overflow.                           //
            //------------------------------------------------------//-----------------------------------------------//

            if (titleLength >= sizeof(titleBuffer))
            {
                titleLength =
                    sizeof(titleBuffer) - 1;
            }

            //------------------------------------------------------//-----------------------------------------------//
            // Skopiuj właściwy tytuł.                                 // Copy the actual title.                      //
            //------------------------------------------------------//-----------------------------------------------//

            memcpy(
                titleBuffer,
                titleStart,
                titleLength);

            //------------------------------------------------------//-----------------------------------------------//
            // Zakończ napis.                                          // Terminate the string.                       //
            //------------------------------------------------------//-----------------------------------------------//

            titleBuffer[titleLength] =
                '\0';

            //------------------------------------------------------//-----------------------------------------------//
            // Zapisz tytuł.                                           // Store the title.                            //
            //------------------------------------------------------//-----------------------------------------------//

            player.title =
                String(titleBuffer);
        }
        else
        {
            //------------------------------------------------------//-----------------------------------------------//
            // Zabezpieczenie dla nietypowego komunikatu.              // Fallback for an unusual message.            //
            //------------------------------------------------------//-----------------------------------------------//

            player.title =
                String(titleStart);

            if (player.title.endsWith(";"))
            {
                player.title.remove(
                    player.title.length() - 1);
            }
        }

               //------------------------------------------------------//-----------------------------------------------//
        // Zgłoś zmianę tytułu oraz ewentualne zmiany              // Report the title change and any changes     //
        // wygenerowane przez osadzony komunikat ELP.              // generated by the embedded ELP message.      //
        //------------------------------------------------------//-----------------------------------------------//

       return ChangeFlags::Title |
          ChangeFlags::CurrentTime |
          ChangeFlags::TotalTime |
          ChangeFlags::Progress |
          embeddedChanges;
    }

//======================================================//-----------------------------------------------//
// Komenda ART - nazwa wykonawcy                           // ART command - artist name                   //
//======================================================//-----------------------------------------------//
//                                                         //                                             //
// Przykład:                                               // Example:                                    //
//                                                         //                                             //
// ART:Kazik;                                              // ART:Kazik;                                  //
//                                                         //                                             //
// Up2Stream może przesyłać ART jako osobny komunikat.     // Up2Stream may send ART as a separate message. //
// Po odebraniu zapisujemy wykonawcę do PlayerState.       // After receiving it, store the artist in PlayerState. //
//                                                         //                                             //
//======================================================//-----------------------------------------------//

if (strncmp(message, "ART:", 4) == 0)
{
    //------------------------------------------------------//-----------------------------------------------//
    // Zapisz nazwę wykonawcy.                                 // Store the artist name.                      //
    //                                                         //                                             //
    // Pomijamy pierwsze cztery znaki:                         // Skip the first four characters:             //
    //                                                         //                                             //
    // A R T :                                                 // A R T :                                     //
    //                                                         //                                             //
    // Pozostała część zawiera właściwą nazwę wykonawcy.       // The remaining part contains the actual artist name. //
    //------------------------------------------------------//-----------------------------------------------//

    player.artist =
        String(message + 4);

    //------------------------------------------------------//-----------------------------------------------//
    // Usuń końcowy znak ';'.                                  // Remove the trailing ';' character.          //
    //                                                         //                                             //
    // Średnik jest separatorem protokołu i nie należy         // The semicolon is a protocol separator and must not //
    // do nazwy wykonawcy.                                     // be part of the artist name.                 //
    //------------------------------------------------------//-----------------------------------------------//

    if (player.artist.endsWith(";"))
    {
        player.artist.remove(
            player.artist.length() - 1);
    }

  
    //------------------------------------------------------//-----------------------------------------------//
    // Poinformuj Display o zmianie wykonawcy.                 // Notify Display about the artist change.     //
    //------------------------------------------------------//-----------------------------------------------//

    return ChangeFlags::Artist;
}

    //======================================================//-----------------------------------------------//
    // Komenda ELP - pozycja odtwarzania                       // ELP command - playback position             //
    //======================================================//-----------------------------------------------//

    //------------------------------------------------------//-----------------------------------------------//
    // Przykład:                                               // Example:                                    //
    //                                                         //                                             //
    // ELP:16337/322986;                                       // ELP:16337/322986;                           //
    //                                                         //                                             //
    // Format:                                                 // Format:                                     //
    //                                                         //                                             //
    // ELP:<elapsed>/<total>;                                  // ELP:<elapsed>/<total>;                      //
    //                                                         //                                             //
    // elapsed - aktualna pozycja w milisekundach              // elapsed - aktualna pozycja w milisekundach  //
    // total   - całkowity czas utworu w milisekundach         // total   - total track duration in milliseconds //
    //------------------------------------------------------//-----------------------------------------------//

    if (strncmp(message, "ELP:", 4) == 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Wskaźnik na pierwszą cyfrę wartości elapsed.            // Pointer to the first digit of the elapsed value. //
        //------------------------------------------------------//-----------------------------------------------//

        const char* separator =
            strchr(message + 4, '/');

        //------------------------------------------------------//-----------------------------------------------//
        // Jeżeli nie znaleziono separatora '/',                   // If the '/' separator was not found,         //
        // komunikat jest niepoprawny.                             // the message is invalid.                     //
        //------------------------------------------------------//-----------------------------------------------//

        if (separator == nullptr)
        {
            return ChangeFlags::None;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Odczytaj elapsed.                                       // Read elapsed.                               //
        //                                                         //                                             //
        // Tymczasowo zapisujemy wartość do lokalnego bufora,      // Temporarily store the value in a local buffer, //
        // ponieważ przed separatorem znajduje się tylko           // because only a fragment of the complete message //
        // fragment całego komunikatu.                             // is located before the separator.            //
        //------------------------------------------------------//-----------------------------------------------//

        char elapsedBuffer[12];

        size_t elapsedLength =
            separator - (message + 4);

        //------------------------------------------------------//-----------------------------------------------//
        // Zabezpieczenie przed przepełnieniem bufora.             // Prevent buffer overflow.                    //
        //------------------------------------------------------//-----------------------------------------------//

        if (elapsedLength >= sizeof(elapsedBuffer))
        {
            return ChangeFlags::None;
        }

        //------------------------------------------------------//-----------------------------------------------//
        // Skopiowanie wartości elapsed.                           // Copy the elapsed value.                     //
        //------------------------------------------------------//-----------------------------------------------//

        memcpy(
            elapsedBuffer,
            message + 4,
            elapsedLength);

        elapsedBuffer[elapsedLength] = '\0';

        //------------------------------------------------------//-----------------------------------------------//
        // Konwersja elapsed.                                      // Convert elapsed.                            //
        //------------------------------------------------------//-----------------------------------------------//

        player.elapsedMs =
            strtoul(
                elapsedBuffer,
                nullptr,
                10);

        //------------------------------------------------------//-----------------------------------------------//
        // Odczyt wartości total.                                  // Read the total value.                       //
        //                                                         //                                             //
        // Za separatorem '/' znajduje się:                        // After the '/' separator there is:           //
        //                                                         //                                             //
        // 322986;                                                 // 322986;                                     //
        //------------------------------------------------------//-----------------------------------------------//

        player.totalMs =
            strtoul(
                separator + 1,
                nullptr,
                10);

        //------------------------------------------------------//-----------------------------------------------//
        // Otrzymaliśmy ELP po zmianie utworu.                     // ELP was received after a track change.      //
        //                                                         //                                             //
        // Oznacza to, że znamy już całkowity czas nowego          // This means that the total duration of the new //
        // utworu. Nie musimy więc dłużej oczekiwać na ELP.        // track is now known, so we no longer need to wait for ELP. //
        //------------------------------------------------------//-----------------------------------------------//

        waitingForTrackELP = false;


  //------------------------------------------------------//-----------------------------------------------//
  // Synchronizacja lokalnego zegara.                        // Synchronize the local clock.                //
  //                                                         //                                             //
  // Zapamiętujemy moment, w którym odebraliśmy              // Store the moment when we received           //
  // aktualną pozycję odtwarzania.                           // the current playback position.              //
  //------------------------------------------------------//-----------------------------------------------//

 player.playbackStartMillis = millis();

        //------------------------------------------------------//-----------------------------------------------//
        // Obliczenie czasu aktualnego w formacie MM:SS.           // Calculate the current time in MM:SS format. //
        //------------------------------------------------------//-----------------------------------------------//

        uint32_t elapsedSeconds =
            player.elapsedMs / 1000;

        uint32_t elapsedMinutes =
            elapsedSeconds / 60;

        elapsedSeconds %= 60;

        //------------------------------------------------------//-----------------------------------------------//
        // Bufor tekstowy dla aktualnego czasu.                    // Text buffer for the current time.           //
        //------------------------------------------------------//-----------------------------------------------//

        static char currentTimeBuffer[12];

        snprintf(
            currentTimeBuffer,
            sizeof(currentTimeBuffer),
            "%02lu:%02lu",
            (unsigned long)elapsedMinutes,
            (unsigned long)elapsedSeconds);

        player.currentTime = currentTimeBuffer;

        //------------------------------------------------------//-----------------------------------------------//
        // Obliczenie całkowitego czasu w formacie MM:SS.          // Calculate the total time in MM:SS format.   //
        //------------------------------------------------------//-----------------------------------------------//

        uint32_t totalSeconds =
            player.totalMs / 1000;

        uint32_t totalMinutes =
            totalSeconds / 60;

        totalSeconds %= 60;

        //------------------------------------------------------//-----------------------------------------------//
        // Bufor tekstowy dla całkowitego czasu.                   // Text buffer for the total time.             //
        //------------------------------------------------------//-----------------------------------------------//

        static char totalTimeBuffer[12];

        snprintf(
            totalTimeBuffer,
            sizeof(totalTimeBuffer),
            "%02lu:%02lu",
            (unsigned long)totalMinutes,
            (unsigned long)totalSeconds);

        player.totalTime = totalTimeBuffer;

        //------------------------------------------------------//-----------------------------------------------//
        // Obliczenie procentowego postępu.                        // Calculate the percentage progress.          //
        //------------------------------------------------------//-----------------------------------------------//

        if (player.totalMs > 0)
        {
            player.progress =
                (int)(
                    (player.elapsedMs * 100UL) /
                    player.totalMs);
        }
        else
        {
            player.progress = 0;
        }
       
              //------------------------------------------------------//-----------------------------------------------//
        // Na tym etapie zgłaszamy zmianę czasu odtwarzania.       // Report the playback-time changes.           //
        //------------------------------------------------------//-----------------------------------------------//

        return ChangeFlags::CurrentTime |
        ChangeFlags::TotalTime |
        ChangeFlags::Progress;
    }

    //======================================================//-----------------------------------------------//
    // Komenda MUT - wyciszenie                                // MUT command - mute                          //
    //======================================================//-----------------------------------------------//

    //------------------------------------------------------//-----------------------------------------------//
    // Przykłady:                                              // Examples:                                    //
    //                                                         //                                             //
    // MUT:0;                                                  // MUT:0;                                      //
    // MUT:1;                                                  // MUT:1;                                      //
    //                                                         //                                             //
    // 0 = brak wyciszenia                                     // 0 = brak wyciszenia                         //
    // 1 = wyciszenie                                          // 1 = wyciszenie                              //
    //------------------------------------------------------//-----------------------------------------------//

    if (strncmp(message, "MUT:", 4) == 0)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Odczytaj stan wyciszenia.                               // Read the mute state.                        //
        //------------------------------------------------------//-----------------------------------------------//

        player.muted = (message[4] == '1');

        
        //------------------------------------------------------//-----------------------------------------------//
        // Poinformuj wyświetlacz o zmianie stanu Mute.            // Notify the display about the Mute state change. //
        //------------------------------------------------------//-----------------------------------------------//

        return ChangeFlags::Mute;
    }

        //======================================================//-----------------------------------------------//
// Komenda TME - aktualny czas urządzenia.               // TME command - current device time.              //
//                                                        //                                               //
// Przykład:                                             // Example:                                      //
//                                                        //                                               //
// TME:2026-08-10 18:37:05 (+1);                         // TME:2026-08-10 18:37:05 (+1);                  //
//                                                        //                                               //
// Parser odczytuje i zapisuje w up2streamTime:           // The parser reads and stores in up2streamTime:  //
// • datę,                                               // • date,                                        //
// • godzinę,                                            // • time,                                        //
// • przesunięcie UTC.                                   // • UTC offset.                                  //
//                                                        //                                               //
// Sam parser nie ustawia RTC Pico.                      // The parser itself does not set the Pico RTC.   //
// Dane są udostępniane przez getTime(), a synchronizacja // The data is exposed through getTime(), while   //
// RTC jest wykonywana przez program główny.              // RTC synchronization is performed by the main   //
//                                                        // program.                                       //
//======================================================//-----------------------------------------------//

    if (strncmp(message, "TME:", 4) == 0)
    {
        int year   = 0;
        int month  = 0;
        int day    = 0;

        int hour   = 0;
        int minute = 0;
        int second = 0;

        int offset = 0;

        //------------------------------------------------------//-----------------------------------------------//
        // Odczyt całej odpowiedzi TME.                            // Read the complete TME response.             //
        //                                                         //                                             //
        // Przykład:                                               // Example:                                    //
        //                                                         //                                             //
        // TME:2026-08-10 18:37:05 (+1);                           // TME:2026-08-10 18:37:05 (+1);               //
        //------------------------------------------------------//-----------------------------------------------//

        int parsed =
            sscanf(
                message,
                "TME:%4d-%2d-%2d %2d:%2d:%2d (%d);",
                &year,
                &month,
                &day,
                &hour,
                &minute,
                &second,
                &offset);

        //------------------------------------------------------//-----------------------------------------------//
        // Sprawdzenie poprawności komunikatu.                     // Validate the message.                       //
        //                                                         //                                             //
        // Musimy otrzymać wszystkie 7 wartości.                   // All 7 values must be received.              //
        //------------------------------------------------------//-----------------------------------------------//

        if (parsed == 7)
        {

            up2streamTime.year = year;
            up2streamTime.month = month;
            up2streamTime.day = day;

            up2streamTime.hour = hour;
            up2streamTime.minute = minute;
            up2streamTime.second = second;

            up2streamTime.utcOffset = offset;

            up2streamTime.valid = true;

            Serial.println(
                "TME PARSED OK");

            //------------------------------------------------------//-----------------------------------------------//
            // Data.                                                   // Date.                                       //
            //------------------------------------------------------//-----------------------------------------------//

            Serial.print("TME DATE = ");

            if (day < 10)
                Serial.print('0');

            Serial.print(day);

            Serial.print('.');

            if (month < 10)
                Serial.print('0');

            Serial.print(month);

            Serial.print('.');

            Serial.println(year);

            //------------------------------------------------------//-----------------------------------------------//
            // Godzina.                                                // Time.                                       //
            //------------------------------------------------------//-----------------------------------------------//

            Serial.print("TME TIME = ");

            if (hour < 10)
                Serial.print('0');

            Serial.print(hour);

            Serial.print(':');

            if (minute < 10)
                Serial.print('0');

            Serial.print(minute);

            Serial.print(':');

            if (second < 10)
                Serial.print('0');

            Serial.println(second);

            //------------------------------------------------------//-----------------------------------------------//
            // Przesunięcie UTC.                                       // UTC offset.                                 //
            //------------------------------------------------------//-----------------------------------------------//

            Serial.print("TME OFFSET = ");

            if (offset >= 0)
                Serial.print('+');

            Serial.println(offset);
        }
        else
        {
            //------------------------------------------------------//-----------------------------------------------//
            // Nie udało się poprawnie sparsować komunikatu.           // The message could not be parsed correctly.  //
            //------------------------------------------------------//-----------------------------------------------//

            Serial.print(
                "TME PARSE ERROR: [");

            Serial.print(message);

            Serial.println(']');
        }

        //------------------------------------------------------//-----------------------------------------------//
        // TME nie zmienia jeszcze PlayerState.                    // TME does not modify PlayerState here.       //
        //------------------------------------------------------//-----------------------------------------------//

        return ChangeFlags::None;
    }

    //======================================================//-----------------------------------------------//
    // Komunikat nie został jeszcze obsłużony.                 // The message is not handled here.            //
    //======================================================//-----------------------------------------------//

    return ChangeFlags::None;
}
