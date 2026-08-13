//==============================================================//-----------------------------------------------//
// Projekt: UP2Stream Display                                // Project: UP2Stream Display                    //
//                                                            //                                               //
// Główny program sterujący wyświetlaczem e-paper dla modułu // Main control program for the e-paper display  //
// Arylic UP2Stream.                                         // for the Arylic UP2Stream module.              //
//==============================================================//-----------------------------------------------//


#include <Arduino.h>

#include "Display.h"
#include "StateComparer.h"
#include "Up2StreamClient.h"
#include "HardwareConfig.h"
#include "PolishTime.h"


//==============================================================//-----------------------------------------------//
// Utworzenie obiektu odpowiedzialnego za obsługę wyświetlacza. // Create the display control object.             //
//==============================================================//-----------------------------------------------//

Display display;


//--------------------------------------------------------------//-----------------------------------------------//
// Komunikacja z modułem Up2Stream.                            // Communication with the Up2Stream module.     //
//--------------------------------------------------------------//-----------------------------------------------//

Up2StreamClient up2stream;


//==============================================================//-----------------------------------------------//
// Informacja o synchronizacji RTC.                            // RTC synchronization status.                  //
//                                                              //                                               //
// RTC jest synchronizowany po uruchomieniu programu oraz       // The RTC is synchronized after startup and      //
// okresowo podczas jego pracy.                                // periodically during operation.               //
//                                                              //                                               //
// Zmienna informuje, czy przynajmniej jedna poprawna            // The variable indicates whether at least one   //
// synchronizacja została już wykonana.                        // valid synchronization has been completed.      //
//==============================================================//-----------------------------------------------//

bool rtcSynchronized = false;


//------------------------------------------------------//-----------------------------------------------//
// Ostatni czas użyty do synchronizacji RTC.           // Last time used to synchronize the RTC.         //
//                                                     //                                               //
// Wartości -1 oznaczają, że RTC nie został jeszcze   // Values of -1 indicate that the RTC has not    //
// zsynchronizowany.                                   // been synchronized.                              //
//------------------------------------------------------//-----------------------------------------------//

int lastRTCSyncYear   = -1;
int lastRTCSyncMonth  = -1;
int lastRTCSyncDay    = -1;
int lastRTCSyncHour   = -1;
int lastRTCSyncMinute = -1;
int lastRTCSyncSecond = -1;


//==============================================================//-----------------------------------------------//
// Okres synchronizacji RTC.                                  // RTC synchronization interval.                  //
//                                                              //                                               //
// RTC będzie okresowo synchronizowany z czasem otrzymanym     // The RTC will be periodically synchronized      //
// z modułu Up2Stream.                                         // with the time received from the Up2Stream      //
//                                                              // module.                                       //
//==============================================================//-----------------------------------------------//

constexpr uint32_t RTC_SYNC_INTERVAL_MS =
    60UL * 10UL * 1000UL;

uint32_t lastRTCSyncRequest = 0;


//==============================================================//-----------------------------------------------//
// Tryb testowy interfejsu.                                    // User interface test mode.                     //
//                                                              //                                               //
// Po włączeniu program wykorzystuje przykładowy stan           // When enabled, the program uses example player //
// odtwarzacza zamiast danych z modułu Up2Stream.               // data instead of data received from Up2Stream. //
//                                                              //                                               //
// W normalnej pracy wartość powinna pozostać false.             // During normal operation, this should remain   //
//                                                              // false.                                        //
//==============================================================//-----------------------------------------------//

constexpr bool USE_TEST_DATA = false;


//==============================================================//-----------------------------------------------//
// Bufory stanu odtwarzacza.                                  // Player state buffers.                        //
//                                                              //                                               //
// previousState                                              // previousState                               //
//      Stan odtwarzacza wyświetlony podczas poprzedniego      //      Player state used during the previous   //
//      odświeżenia.                                           //      display update.                         //
//                                                              //                                               //
// currentState                                               // currentState                                //
//      Aktualny stan odebrany z modułu UP2Stream.             //      Current state received from Up2Stream.  //
//==============================================================//-----------------------------------------------//

PlayerState previousState;
PlayerState currentState;


//==============================================================//-----------------------------------------------//
// Wczytanie przykładowych danych.                            // Load example data.                           //
//                                                              //                                               //
// Funkcja może być wykorzystana podczas projektowania          // The function can be used while developing    //
// interfejsu użytkownika bez podłączonego modułu Up2Stream.   // the user interface without a connected        //
//                                                              // Up2Stream module.                              //
//                                                              //                                               //
// Funkcja jest wykonywana tylko wtedy, gdy                     // The function is executed only when             //
// USE_TEST_DATA ma wartość true.                              // USE_TEST_DATA is set to true.                  //
//==============================================================//-----------------------------------------------//

void loadTestData(PlayerState& state)
{
    state.source = "--";

    state.artist = "...";

    state.title =
        "...";

    state.currentTime = "02:15";

    state.totalTime = "08:26";

    state.progress = 35;

    state.volume = 38;

    state.playing = true;
}


//==============================================================//-----------------------------------------------//
// Funkcja setup()                                            // setup() function                             //
//                                                              //                                               //
// Wykonywana jednorazowo po uruchomieniu mikrokontrolera.    // Executed once after the microcontroller      //
//                                                              // starts.                                      //
//==============================================================//-----------------------------------------------//

void setup()
{
    //----------------------------------------------------------//-----------------------------------------------//
    // Uruchomienie portu szeregowego.                         // Start the serial port.                       //
    // Wykorzystywany podczas uruchamiania oraz debugowania.   // Used during startup and debugging.           //
    //----------------------------------------------------------//-----------------------------------------------//

    while (!Serial)
        delay(10);



    //----------------------------------------------------------//-----------------------------------------------//
    // Inicjalizacja wyświetlacza.                             // Initialize the display.                     //
    //----------------------------------------------------------//-----------------------------------------------//

    display.begin();


    //----------------------------------------------------------//-----------------------------------------------//
    // Konfiguracja portu UART wykorzystywanego do komunikacji // Configure the UART port used for             //
    // z modułem Up2Stream.                                    // communication with the Up2Stream module.    //
    //----------------------------------------------------------//-----------------------------------------------//

    UP2STREAM_SERIAL.setTX(
        UP2STREAM_UART_TX_PIN);

    UP2STREAM_SERIAL.setRX(
        UP2STREAM_UART_RX_PIN);


    //----------------------------------------------------------//-----------------------------------------------//
    // Zwiększenie sprzętowego bufora odbiorczego UART.        // Increase the UART hardware receive buffer.  //
    //                                                          //                                               //
    // Domyślny FIFO UART w Arduino-Pico ma 32 bajty.          // The default Arduino-Pico UART FIFO is       //
    // Zwiększamy go do 128 bajtów, aby ograniczyć ryzyko      // 32 bytes. We increase it to 128 bytes to    //
    // utraty znaków podczas odbioru dłuższych komunikatów,   // reduce the risk of losing characters while   //
    // szczególnie podczas pracy wyświetlacza e-paper.         // receiving longer messages, especially while  //
    //                                                          // the e-paper display is operating.             //
    // UWAGA:                                                  // NOTE:                                         //
    // setFIFOSize() musi zostać wywołane przed begin().       // setFIFOSize() must be called before begin().  //
    //----------------------------------------------------------//-----------------------------------------------//

    UP2STREAM_SERIAL.setFIFOSize(128);

    UP2STREAM_SERIAL.begin(115200);


    //----------------------------------------------------------//-----------------------------------------------//
    // Inicjalizacja klienta Up2Stream.                        // Initialize the Up2Stream client.             //
    //----------------------------------------------------------//-----------------------------------------------//

    up2stream.begin(
        UP2STREAM_SERIAL);


//==============================================================//-----------------------------------------------//
// Pierwsze zapytanie o aktualny czas z Up2Stream.             // Initial request for the current time from    //
//                                                              // Up2Stream.                                    //
//                                                              //                                               //
// Zapytanie inicjuje pierwszą synchronizację RTC.              // The request initiates the first RTC          //
// Kolejne zapytania są wysyłane okresowo w funkcji loop().    // synchronization. Further requests are sent  //
//                                                              // periodically from loop().                     //
//==============================================================//-----------------------------------------------//

    delay(1000);

    up2stream.query("STA;");


    //----------------------------------------------------------//-----------------------------------------------//
    // Krótkie opóźnienie umożliwiające obejrzenie ekranu       // Short delay allowing the startup screen to   //
    // startowego.                                             // remain visible.                               //
    //----------------------------------------------------------//-----------------------------------------------//

    delay(3000);

    up2stream.query("TME;");

    lastRTCSyncRequest = millis();


//--------------------------------------------------------------//-----------------------------------------------//
// Pobranie aktualnego źródła odtwarzania.                     // Request the current playback source.         //
//                                                              //                                               //
// Up2Stream odpowie komunikatem:                              // Up2Stream will respond with a message such as://
//                                                              //                                               //
//     SRC:NET;                                                //     SRC:NET;                                   //
//     SRC:BT;                                                 //     SRC:BT;                                    //
//     SRC:LINE-IN;                                            //     SRC:LINE-IN;                               //
//     itd.                                                    //     etc.                                       //
//                                                              //                                               //
// Odpowiedź zostanie odebrana i przetworzona podczas          // The response will be received and processed  //
// kolejnego wywołania up2stream.update().                     // during the next call to up2stream.update().  //
//--------------------------------------------------------------//-----------------------------------------------//

    up2stream.query("SRC;");


//----------------------------------------------------------//-----------------------------------------------//
// Przygotowanie przykładowych danych.                       // Prepare example data.                         //
//                                                              //                                               //
// Jeżeli aktywny jest tryb testowy, stan odtwarzacza          // If test mode is enabled, the player state     //
// zostanie uzupełniony przykładowymi wartościami.              // is initialized with example values.           //
//                                                              //                                               //
// W normalnej pracy dane pochodzą z modułu Up2Stream.         // During normal operation, data comes from      //
//                                                              // the Up2Stream module.                          //
//----------------------------------------------------------//-----------------------------------------------//

    if (USE_TEST_DATA)
    {
        loadTestData(currentState);
    }


//==========================================================//-----------------------------------------------//
// Porównanie poprzedniego oraz aktualnego stanu            // Compare the previous and current player state //
// odtwarzacza.                                             // values.                                       //
//                                                          //                                               //
// Funkcja compare() zwraca zestaw flag określających,     // The compare() function returns a set of flags //
// które elementy stanu odtwarzacza uległy zmianie.         // indicating which player state elements have   //
//                                                          // changed.                                      //
//==========================================================//-----------------------------------------------//

    ChangeFlags changes =
        StateComparer::compare(previousState, currentState);


//==========================================================//-----------------------------------------------//
// Wyświetlenie aktualnego stanu odtwarzacza.               // Display the current player state.             //
//==========================================================//-----------------------------------------------//

    display.update(currentState, changes);


//==========================================================//-----------------------------------------------//
// Zapamiętanie aktualnego stanu.                           // Store the current player state.               //
//                                                          //                                               //
// Podczas kolejnego odświeżenia będzie on traktowany jako  // During the next update it will be treated as   //
// stan poprzedni.                                          // the previous state.                            //
//==========================================================//-----------------------------------------------//

    previousState = currentState;
}


//==============================================================//-----------------------------------------------//
// Funkcja loop()                                             // loop() function                              //
//                                                              //                                               //
// Główna pętla programu wykonywana w sposób ciągły.           // Main program loop executed continuously.     //
//                                                              //                                               //
// Obecnie zawiera podstawową obsługę komunikacji UART,        // It currently contains the basic UART         //
// synchronizacji RTC oraz aktualizacji wyświetlacza.          // communication, RTC synchronization and       //
//                                                              // display update handling.                     //
//==============================================================//-----------------------------------------------//

void loop()
{
    //----------------------------------------------------------//-----------------------------------------------//
    // Odczyt danych z modułu Up2Stream.                       // Read data from the Up2Stream module.         //
    //----------------------------------------------------------//-----------------------------------------------//

    ChangeFlags changes =
        up2stream.update(currentState);


//==============================================================//-----------------------------------------------//
// Wyjście z trybu standby.                                  // Exit from standby mode.                      //
//                                                              //                                               //
// Po otrzymaniu SYS:ON ponownie pytamy Up2Stream o aktualne  // After receiving SYS:ON, request the current   //
// źródło.                                                    // source from Up2Stream again.                  //
//                                                              //                                               //
// Nie pytamy o VND.                                          // VND is not requested.                         //
// VND jest wysyłane przez Up2Stream spontanicznie.            // VND is sent spontaneously by Up2Stream.      //
//==============================================================//-----------------------------------------------//

    if ((changes & ChangeFlags::Standby)
        != ChangeFlags::None &&
        !currentState.standby)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Pobranie aktualnego źródła.                         // Request the current source.                  //
        //------------------------------------------------------//-----------------------------------------------//

        up2stream.query("SRC;");
    }


//==============================================================//-----------------------------------------------//
// Synchronizacja RTC.                                         // RTC synchronization.                          //
//                                                              //                                               //
// Odpowiedź TME jest przeliczana na czas polski i używana      // The TME response is converted to Polish time   //
// do ustawienia zegara RTC Raspberry Pi Pico.                  // and used to set the Raspberry Pi Pico RTC.     //
//                                                              //                                               //
// Pierwsza synchronizacja odbywa się po uruchomieniu,          // The first synchronization takes place after    //
// a kolejne są wykonywane okresowo.                            // startup, with subsequent synchronizations       //
//                                                              // performed periodically.                        //
//==============================================================//-----------------------------------------------//

    const Up2StreamTime& up2streamTime =
        up2stream.getTime();

    if (up2streamTime.valid)
    {
        //------------------------------------------------------//-----------------------------------------------//
        // Sprawdź, czy otrzymany czas różni się od czasu      // Check whether the received time differs from  //
        // użytego podczas ostatniej synchronizacji.           // the time used for the last synchronization.    //
        //------------------------------------------------------//-----------------------------------------------//

        bool newTime =
            !rtcSynchronized ||
            up2streamTime.year   != lastRTCSyncYear ||
            up2streamTime.month  != lastRTCSyncMonth ||
            up2streamTime.day    != lastRTCSyncDay ||
            up2streamTime.hour   != lastRTCSyncHour ||
            up2streamTime.minute != lastRTCSyncMinute ||
            up2streamTime.second != lastRTCSyncSecond;


        //------------------------------------------------------//-----------------------------------------------//
        // Synchronizuj RTC tylko po otrzymaniu nowego czasu.   // Synchronize the RTC only after receiving new    //
        //                                                     // time data.                                     //
        //------------------------------------------------------//-----------------------------------------------//

        if (newTime)
        {
            PolishTime polishTime =
                convertToPolishTime(
                    up2streamTime);

            if (polishTime.valid)
            {
                display.setRTC(
                    polishTime);

                //------------------------------------------------------//-----------------------------------------------//
                // Zapamiętaj czas wykorzystany do synchronizacji.       // Remember the time used for synchronization.  //
                //------------------------------------------------------//-----------------------------------------------//

                lastRTCSyncYear =
                    up2streamTime.year;

                lastRTCSyncMonth =
                    up2streamTime.month;

                lastRTCSyncDay =
                    up2streamTime.day;

                lastRTCSyncHour =
                    up2streamTime.hour;

                lastRTCSyncMinute =
                    up2streamTime.minute;

                lastRTCSyncSecond =
                    up2streamTime.second;

                rtcSynchronized = true;
            }
        }
    }


//==============================================================//-----------------------------------------------//
// Okresowe zapytanie o aktualny czas.                         // Periodic request for the current time.        //
//==============================================================//-----------------------------------------------//

    if (millis() - lastRTCSyncRequest >= RTC_SYNC_INTERVAL_MS)
    {
        //----------------------------------------------------------//-----------------------------------------------//
        // Zapytanie o aktualny czas z modułu Up2Stream.            // Request the current time from the Up2Stream   //
        //                                                          // module.                                       //
        //----------------------------------------------------------//-----------------------------------------------//

        up2stream.query("TME;");

        lastRTCSyncRequest = millis();
    }


    //----------------------------------------------------------//-----------------------------------------------//
    // Aktualizacja wyświetlacza.                             // Update the display.                          //
    //----------------------------------------------------------//-----------------------------------------------//

    display.update(
        currentState,
        changes);


    //----------------------------------------------------------//-----------------------------------------------//
    // Ograniczenie częstotliwości odświeżania.               // Limit the update frequency.                  //
    //----------------------------------------------------------//-----------------------------------------------//

    delay(40);
}