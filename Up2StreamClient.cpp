//==============================================================
// Projekt : UP2Stream Display
// Plik    : Up2StreamClient.cpp
//
// Opis:
//
// Implementacja klasy odpowiedzialnej za komunikację UART
// z modułem Arylic Up2Stream.
//
// Implementacja obejmuje:
//
// • konstruktor,
// • inicjalizację UART,
// • odbiór komunikatów UART,
// • parser PLA / VOL / SRC / TIT / ELP / MUT,
// • lokalne odmierzanie czasu odtwarzania,
// • obsługę sklejonych komunikatów TIT + ELP.
//==============================================================

#include "Up2StreamClient.h"


//==============================================================
// Konstruktor.
//
// Zeruje wskaźnik do portu UART.
//==============================================================

Up2StreamClient::Up2StreamClient()
{
    //----------------------------------------------------------
    // Port UART nie został jeszcze przypisany.
    //----------------------------------------------------------

    uart = nullptr;

    //----------------------------------------------------------
    // Początek bufora odbiorczego.
    //----------------------------------------------------------

    rxPosition = 0;

    //----------------------------------------------------------
    // Na początku nie oczekujemy na ELP nowego utworu.
    //----------------------------------------------------------

    waitingForTrackELP = false;

    rxBuffer[0] = '\0';
}

//==============================================================
// Inicjalizacja.
//
// Zapamiętuje port UART wykorzystywany do komunikacji
// z modułem Up2Stream.
//
// Sama konfiguracja prędkości transmisji będzie wykonywana
// w programie głównym.
//==============================================================

void Up2StreamClient::begin(HardwareSerial& serial)
{
    uart = &serial;
}



//==============================================================
// Aktualizacja.
//
// Odczytuje wszystkie znaki oczekujące w buforze UART.
//
// Na obecnym etapie:
//
// • odbiera znaki,
// • zapisuje je do bufora,
// • wykrywa koniec komunikatu (';').
//
// Analiza odebranego komunikatu zostanie dodana
// w następnym kroku.
//==============================================================

ChangeFlags Up2StreamClient::update(PlayerState& player)
{
    //----------------------------------------------------------
    // Jeżeli UART nie został jeszcze zainicjalizowany,
    // nie wykonujemy żadnych działań.
    //----------------------------------------------------------

    if (uart == nullptr)
    {
        return ChangeFlags::None;
    }



    //----------------------------------------------------------
    // Zestaw zmian wykrytych podczas tego wywołania update().
    //
    // Nie zwracamy już natychmiast po odebraniu komunikatu.
    // Dzięki temu w jednym przebiegu możemy:
    //
    // • odebrać komunikat UART,
    // • zaktualizować PlayerState,
    // • zaktualizować lokalny zegar,
    // • zwrócić kompletny zestaw zmian.
    //----------------------------------------------------------

    ChangeFlags changes = ChangeFlags::None;

    //----------------------------------------------------------
    // Odczytaj wszystkie dostępne znaki z UART.
    //----------------------------------------------------------

    while (uart->available())
    {
        //------------------------------------------------------
        // Pobierz jeden znak.
        //------------------------------------------------------

        char c = uart->read();

        //------------------------------------------------------
        // Zabezpieczenie przed przepełnieniem bufora.
        //------------------------------------------------------

        if (rxPosition >= RX_BUFFER_SIZE - 1)
        {
            rxPosition = 0;
        }

        //------------------------------------------------------
        // Dopisz znak do bufora.
        //------------------------------------------------------

        rxBuffer[rxPosition++] = c;

        //------------------------------------------------------
        // Zakończenie napisu.
        //------------------------------------------------------

        rxBuffer[rxPosition] = '\0';


        //------------------------------------------------------
        // Czy odebrano kompletny komunikat?
        //------------------------------------------------------

        if (c == ';')
        {
            //--------------------------------------------------
            // Przetwórz komunikat.
            //--------------------------------------------------


            
            ChangeFlags messageChanges =
                processMessage(
                    rxBuffer,
                    player);

            //--------------------------------------------------
            // Dodaj wykryte zmiany do wspólnego zestawu.
            //--------------------------------------------------

            changes =
                changes | messageChanges;



            //--------------------------------------------------
            // Wyczyść bufor.
            //--------------------------------------------------

            rxPosition = 0;

            rxBuffer[0] = '\0';
        }
    }

    //----------------------------------------------------------
    // Lokalny zegar odtwarzania.
    //
    // ELP synchronizuje pozycję.
    // Pomiędzy komunikatami ELP czas odmierzamy lokalnie.
    //----------------------------------------------------------

    static int lastDisplayedSecond = -1;

    if (player.playing &&
        player.totalMs > 0)
    {
        //------------------------------------------------------
        // Aktualny czas systemowy.
        //------------------------------------------------------

        uint32_t now = millis();

     //------------------------------------------------------
     // Czas, który upłynął od ostatniego punktu odniesienia.
     //------------------------------------------------------
   

        uint32_t delta =
           now - player.playbackStartMillis;

    //------------------------------------------------------
    // Aktualna pozycja utworu.
    //------------------------------------------------------       

        uint32_t localElapsed =
                 player.elapsedMs + delta;

    //------------------------------------------------------
    // Nowy punkt odniesienia.
    //------------------------------------------------------


        player.playbackStartMillis = now;


        //------------------------------------------------------
        // Nie przekraczaj końca utworu.
        //------------------------------------------------------

        if (localElapsed > player.totalMs)
        {
            localElapsed = player.totalMs;
        }

        //------------------------------------------------------
        // Sekunda przed formatowaniem MM:SS.
        //
        // UWAGA:
        // przechowujemy całkowitą liczbę sekund,
        // np. 135 dla 02:15.
        //------------------------------------------------------

        uint32_t totalElapsedSeconds =
            localElapsed / 1000;

        //------------------------------------------------------
        // Sprawdź, czy zmieniła się wyświetlana sekunda.
        //------------------------------------------------------

        bool secondChanged =
            ((int)totalElapsedSeconds !=
             lastDisplayedSecond);

        //------------------------------------------------------
        // Aktualizacja czasu MM:SS.
        //------------------------------------------------------

        uint32_t displaySeconds =
            totalElapsedSeconds;

        uint32_t displayMinutes =
            displaySeconds / 60;

        displaySeconds %= 60;

        //------------------------------------------------------
        // Bufor aktualnego czasu.
        //------------------------------------------------------

        static char currentTimeBuffer[12];

        snprintf(
            currentTimeBuffer,
            sizeof(currentTimeBuffer),
            "%02lu:%02lu",
            (unsigned long)displayMinutes,
            (unsigned long)displaySeconds);

        //------------------------------------------------------
        // Zapisz aktualną pozycję.
        //------------------------------------------------------

        player.elapsedMs = localElapsed;

        player.currentTime =
            currentTimeBuffer;

        //------------------------------------------------------
        // Aktualizacja postępu.
        //------------------------------------------------------

        player.progress =
            (int)(
                (player.elapsedMs * 100UL) /
                player.totalMs);

        //------------------------------------------------------
        // Jeżeli zmieniła się sekunda,
        // zgłoś zmianę do Display.
        //------------------------------------------------------

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

    //----------------------------------------------------------
    // Zwróć wszystkie zmiany wykryte podczas tego przebiegu.
    //----------------------------------------------------------

    return changes;
}
//==============================================================
// Analiza pojedynczego komunikatu.
//
// Na obecnym etapie obsługiwana jest wyłącznie komenda PLA.
//
// Przykłady:
//
// PLA:0;
// PLA:1;
//==============================================================

//==============================================================
// Analiza pojedynczego komunikatu.
//
// Obsługiwane komendy:
//
// PLA:0;
// PLA:1;
//
// VOL:0;
// VOL:100;
//==============================================================

ChangeFlags Up2StreamClient::processMessage(
    const char* message,
    PlayerState& player)
{
    //----------------------------------------------------------
    // Pomijanie znaków CR i LF znajdujących się na początku
    // komunikatu.
    //
    // Up2Stream może wysyłać komunikaty poprzedzone znakami:
    //
    // CR = '\r'
    // LF = '\n'
    //
    // Dzięki temu parser może poprawnie rozpoznać np.:
    //
    // \rVOL:80;
    //
    // jako:
    //
    // VOL:80;
    //----------------------------------------------------------

    while (*message == '\r' ||
           *message == '\n')
    {
        message++;
    }

        //==========================================================
    // Komenda PLAY / PAUSE
    //==========================================================

    //----------------------------------------------------------
    // Przykłady:
    //
    // PLA:0;
    // PLA:1;
    //----------------------------------------------------------

        if (strncmp(message, "PLA:", 4) == 0)
    {
        //------------------------------------------------------
        // Odczytaj stan odtwarzania.
        //
        // '0' = pauza
        // '1' = odtwarzanie
        //------------------------------------------------------

        bool newPlaying =
            (message[4] == '1');

        //------------------------------------------------------
        // PAUSE
        //------------------------------------------------------

        if (!newPlaying)
        {
            //--------------------------------------------------
            // Jeżeli właśnie przechodzimy z PLAY do PAUSE,
            // zapisz aktualną pozycję odtwarzania.
            //
            // Dzięki temu po wznowieniu nie zaczniemy
            // odmierzać czasu od poprzedniego punktu
            // synchronizacji ELP.
            //--------------------------------------------------

            if (player.playing &&
                player.totalMs > 0)
            {
                uint32_t now = millis();

                uint32_t delta =
                    now - player.playbackStartMillis;

                uint32_t pausedElapsed =
                    player.elapsedMs + delta;

                //------------------------------------------------
                // Nie przekraczaj końca utworu.
                //------------------------------------------------

                if (pausedElapsed > player.totalMs)
                {
                    pausedElapsed =
                        player.totalMs;
                }

                player.elapsedMs =
                    pausedElapsed;
            }

            //--------------------------------------------------
            // Ustaw stan PAUSE.
            //--------------------------------------------------

            player.playing = false;
        }

        //------------------------------------------------------
        // PLAY
        //------------------------------------------------------

        else
        {
            //--------------------------------------------------
            // Ustaw nowy punkt odniesienia dla lokalnego zegara.
            //
            // Od tej chwili będziemy dodawać czas do aktualnego
            // elapsedMs.
            //--------------------------------------------------

            player.playbackStartMillis =
                millis();

            //--------------------------------------------------
            // Ustaw stan PLAY.
            //--------------------------------------------------

            player.playing = true;
        }

        //------------------------------------------------------
        // Poinformuj wyświetlacz o zmianie stanu odtwarzania.
        //------------------------------------------------------

        return ChangeFlags::PlayState;
    }


    //==========================================================
    // Komenda VOL - poziom głośności
    //==========================================================

    //----------------------------------------------------------
    // Przykład:
    //
    // VOL:87;
    //----------------------------------------------------------

    if (strncmp(message, "VOL:", 4) == 0)
    {
        //------------------------------------------------------
        // Odczytaj poziom głośności.
        //------------------------------------------------------

        player.volume = atoi(message + 4);

        //------------------------------------------------------
        // Poinformuj wyświetlacz o zmianie poziomu głośności.
        //------------------------------------------------------

        return ChangeFlags::Volume;
    }


    //==========================================================
    // Komenda SRC - źródło odtwarzania
    //==========================================================

    //----------------------------------------------------------
    // Przykłady:
    //
    // SRC:NET;
    // SRC:LINE-IN;
    // SRC:USB;
    // SRC:BT;
    //----------------------------------------------------------

    if (strncmp(message, "SRC:", 4) == 0)
    {
        //------------------------------------------------------
        // Zapisz nazwę źródła.
        //
        // Pomijamy pierwsze cztery znaki:
        //
        // S R C :
        //
        // Pozostała część zawiera właściwą wartość.
        //------------------------------------------------------

        player.source = String(message + 4);

        //------------------------------------------------------
        // Usuń końcowy znak ';', ponieważ nie jest częścią
        // nazwy źródła.
        //------------------------------------------------------

        if (player.source.endsWith(";"))
        {
            player.source.remove(
                player.source.length() - 1);
        }

        //------------------------------------------------------
        // Poinformuj wyświetlacz o zmianie źródła.
        //------------------------------------------------------

        return ChangeFlags::Source;
    }

//==========================================================
// Komenda TIT - tytuł utworu
//==========================================================
//
// Normalny komunikat:
//
// TIT:Mandalay;
//
// Up2Stream może jednak skleić kolejne pola:
//
// TIT:Historia choALB:Gra?;
//
// albo:
//
// TIT:Historia choART:El Dupa;
//
// Dlatego szukamy początku następnego pola
// i kończymy tytuł przed ART: lub ALB:.
//==========================================================

    if (strncmp(message, "TIT:", 4) == 0)
    {



        //------------------------------------------------------
        // Początek właściwego tytułu.
        //------------------------------------------------------

        const char* titleStart =
            message + 4;

        //------------------------------------------------------
        // Rozpoczęcie obsługi nowego utworu.
        //
        // Sam komunikat TIT oznacza, że zmienił się utwór,
        // ale nie znamy jeszcze jego całkowitego czasu.
        //
        // Czas całkowity zostanie uzupełniony dopiero po
        // odebraniu pierwszego ELP dla nowego utworu.
        //------------------------------------------------------

        waitingForTrackELP = true;

        //------------------------------------------------------
        // Zerujemy lokalny stan czasu.
        //
        // Nie wolno pozostawić tutaj wartości poprzedniego
        // utworu, ponieważ przez pewien czas może ona być
        // jeszcze wyświetlana.
        //------------------------------------------------------

        player.elapsedMs = 0;

        player.currentTime =
            "00:00";

        player.totalTime =
            "--:--";

        player.progress =
            0;    

        //------------------------------------------------------
        // Up2Stream może skleić TIT z kolejnym komunikatem ELP.
        //
        // Przykład:
        //
        // TIT:Ballada oELP:31880/137426;
        //
        // Tytuł kończy się więc przed "ELP:".
        // Fragment ELP przekazujemy ponownie do parsera,
        // aby jednocześnie zaktualizować czas odtwarzania.
        //------------------------------------------------------

        const char* embeddedELP =
            strstr(titleStart, "ELP:");

        //------------------------------------------------------
        // Zmiany wygenerowane przez osadzony komunikat ELP.
        //------------------------------------------------------

        ChangeFlags embeddedChanges =
            ChangeFlags::None;

        if (embeddedELP != nullptr)
        {
           

            //--------------------------------------------------
            // Przetwórz np.:
            //
            // ELP:31880/137426;
            //--------------------------------------------------

            embeddedChanges =
                processMessage(
                    embeddedELP,
                    player);
        }

        //------------------------------------------------------
        // Ustal początkowy koniec tytułu.
        //
        // W normalnym komunikacie jest nim średnik.
        //------------------------------------------------------

        const char* titleEnd =
            strchr(titleStart, ';');

        //------------------------------------------------------
        // Poszukaj kolejnych pól sklejonych z tytułem.
        //------------------------------------------------------

        const char* artistStart =
            strstr(titleStart, "ART:");

        const char* albumStart =
            strstr(titleStart, "ALB:");

        //------------------------------------------------------
        // ART: rozpoczyna kolejne pole.
        //------------------------------------------------------

        if (artistStart != nullptr &&
            (titleEnd == nullptr ||
             artistStart < titleEnd))
        {
            titleEnd =
                artistStart;
        }

        //------------------------------------------------------
        // ALB: rozpoczyna kolejne pole.
        //------------------------------------------------------

        if (albumStart != nullptr &&
            (titleEnd == nullptr ||
             albumStart < titleEnd))
        {
            titleEnd =
                albumStart;
        }

        //------------------------------------------------------
        // ELP: rozpoczyna sklejony komunikat pozycji.
        //
        // Jest to istotne dla przypadku:
        //
        // TIT:Ballada oELP:31880/137426;
        //
        // Bez tego fragment "ELP:..." zostałby dopisany
        // do tytułu.
        //------------------------------------------------------

        if (embeddedELP != nullptr &&
            (titleEnd == nullptr ||
             embeddedELP < titleEnd))
        {
            titleEnd =
                embeddedELP;
        }

        //------------------------------------------------------
        // Jeżeli znaleziono granicę tytułu, skopiuj tylko
        // właściwy fragment.
        //------------------------------------------------------

        if (titleEnd != nullptr)
        {
            //--------------------------------------------------
            // Oblicz długość właściwego tytułu.
            //--------------------------------------------------

            size_t titleLength =
                titleEnd - titleStart;

            //--------------------------------------------------
            // Tymczasowy bufor tytułu.
            //--------------------------------------------------

            char titleBuffer[RX_BUFFER_SIZE];

            //--------------------------------------------------
            // Zabezpieczenie przed przepełnieniem.
            //--------------------------------------------------

            if (titleLength >= sizeof(titleBuffer))
            {
                titleLength =
                    sizeof(titleBuffer) - 1;
            }

            //--------------------------------------------------
            // Skopiuj właściwy tytuł.
            //--------------------------------------------------

            memcpy(
                titleBuffer,
                titleStart,
                titleLength);

            //--------------------------------------------------
            // Zakończ napis.
            //--------------------------------------------------

            titleBuffer[titleLength] =
                '\0';

            //--------------------------------------------------
            // Zapisz tytuł.
            //--------------------------------------------------

            player.title =
                String(titleBuffer);
        }
        else
        {
            //--------------------------------------------------
            // Zabezpieczenie dla nietypowego komunikatu.
            //--------------------------------------------------

            player.title =
                String(titleStart);

            if (player.title.endsWith(";"))
            {
                player.title.remove(
                    player.title.length() - 1);
            }
        }

               //------------------------------------------------------
        // Zgłoś zmianę tytułu oraz ewentualne zmiany
        // wygenerowane przez osadzony komunikat ELP.
        //------------------------------------------------------

       return ChangeFlags::Title |
          ChangeFlags::CurrentTime |
          ChangeFlags::TotalTime |
          ChangeFlags::Progress |
          embeddedChanges;
    }

//==========================================================
// Komenda ART - nazwa wykonawcy
//==========================================================
//
// Przykład:
//
// ART:Kazik;
//
// Up2Stream może przesyłać ART jako osobny komunikat.
// Po odebraniu zapisujemy wykonawcę do PlayerState.
//
//==========================================================

if (strncmp(message, "ART:", 4) == 0)
{
    //------------------------------------------------------
    // Zapisz nazwę wykonawcy.
    //
    // Pomijamy pierwsze cztery znaki:
    //
    // A R T :
    //
    // Pozostała część zawiera właściwą nazwę wykonawcy.
    //------------------------------------------------------

    player.artist =
        String(message + 4);

    //------------------------------------------------------
    // Usuń końcowy znak ';'.
    //
    // Średnik jest separatorem protokołu i nie należy
    // do nazwy wykonawcy.
    //------------------------------------------------------

    if (player.artist.endsWith(";"))
    {
        player.artist.remove(
            player.artist.length() - 1);
    }

  
    //------------------------------------------------------
    // Poinformuj Display o zmianie wykonawcy.
    //------------------------------------------------------

    return ChangeFlags::Artist;
}

    //==========================================================
    // Komenda ELP - pozycja odtwarzania
    //==========================================================

    //----------------------------------------------------------
    // Przykład:
    //
    // ELP:16337/322986;
    //
    // Format:
    //
    // ELP:<elapsed>/<total>;
    //
    // elapsed - aktualna pozycja w milisekundach
    // total   - całkowity czas utworu w milisekundach
    //----------------------------------------------------------

    if (strncmp(message, "ELP:", 4) == 0)
    {
        //------------------------------------------------------
        // Wskaźnik na pierwszą cyfrę wartości elapsed.
        //------------------------------------------------------

        const char* separator =
            strchr(message + 4, '/');

        //------------------------------------------------------
        // Jeżeli nie znaleziono separatora '/',
        // komunikat jest niepoprawny.
        //------------------------------------------------------

        if (separator == nullptr)
        {
            return ChangeFlags::None;
        }

        //------------------------------------------------------
        // Odczytaj elapsed.
        //
        // Tymczasowo zapisujemy wartość do lokalnego bufora,
        // ponieważ przed separatorem znajduje się tylko
        // fragment całego komunikatu.
        //------------------------------------------------------

        char elapsedBuffer[12];

        size_t elapsedLength =
            separator - (message + 4);

        //------------------------------------------------------
        // Zabezpieczenie przed przepełnieniem bufora.
        //------------------------------------------------------

        if (elapsedLength >= sizeof(elapsedBuffer))
        {
            return ChangeFlags::None;
        }

        //------------------------------------------------------
        // Skopiowanie wartości elapsed.
        //------------------------------------------------------

        memcpy(
            elapsedBuffer,
            message + 4,
            elapsedLength);

        elapsedBuffer[elapsedLength] = '\0';

        //------------------------------------------------------
        // Konwersja elapsed.
        //------------------------------------------------------

        player.elapsedMs =
            strtoul(
                elapsedBuffer,
                nullptr,
                10);

        //------------------------------------------------------
        // Odczyt wartości total.
        //
        // Za separatorem '/' znajduje się:
        //
        // 322986;
        //------------------------------------------------------

        player.totalMs =
            strtoul(
                separator + 1,
                nullptr,
                10);

        //------------------------------------------------------
        // Otrzymaliśmy ELP po zmianie utworu.
        //
        // Oznacza to, że znamy już całkowity czas nowego
        // utworu. Nie musimy więc dłużej oczekiwać na ELP.
        //------------------------------------------------------

        waitingForTrackELP = false;


  //------------------------------------------------------
  // Synchronizacja lokalnego zegara.
  //
  // Zapamiętujemy moment, w którym odebraliśmy
  // aktualną pozycję odtwarzania.
  //------------------------------------------------------

 player.playbackStartMillis = millis();

        //------------------------------------------------------
        // Obliczenie czasu aktualnego w formacie MM:SS.
        //------------------------------------------------------

        uint32_t elapsedSeconds =
            player.elapsedMs / 1000;

        uint32_t elapsedMinutes =
            elapsedSeconds / 60;

        elapsedSeconds %= 60;

        //------------------------------------------------------
        // Bufor tekstowy dla aktualnego czasu.
        //------------------------------------------------------

        static char currentTimeBuffer[12];

        snprintf(
            currentTimeBuffer,
            sizeof(currentTimeBuffer),
            "%02lu:%02lu",
            (unsigned long)elapsedMinutes,
            (unsigned long)elapsedSeconds);

        player.currentTime = currentTimeBuffer;

        //------------------------------------------------------
        // Obliczenie całkowitego czasu w formacie MM:SS.
        //------------------------------------------------------

        uint32_t totalSeconds =
            player.totalMs / 1000;

        uint32_t totalMinutes =
            totalSeconds / 60;

        totalSeconds %= 60;

        //------------------------------------------------------
        // Bufor tekstowy dla całkowitego czasu.
        //------------------------------------------------------

        static char totalTimeBuffer[12];

        snprintf(
            totalTimeBuffer,
            sizeof(totalTimeBuffer),
            "%02lu:%02lu",
            (unsigned long)totalMinutes,
            (unsigned long)totalSeconds);

        player.totalTime = totalTimeBuffer;

        //------------------------------------------------------
        // Obliczenie procentowego postępu.
        //------------------------------------------------------

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
       
              //------------------------------------------------------
        // Na tym etapie zgłaszamy zmianę czasu odtwarzania.
        //------------------------------------------------------

        return ChangeFlags::CurrentTime |
        ChangeFlags::TotalTime |
        ChangeFlags::Progress;
    }

    //==========================================================
    // Komenda MUT - wyciszenie
    //==========================================================

    //----------------------------------------------------------
    // Przykłady:
    //
    // MUT:0;
    // MUT:1;
    //
    // 0 = brak wyciszenia
    // 1 = wyciszenie
    //----------------------------------------------------------

    if (strncmp(message, "MUT:", 4) == 0)
    {
        //------------------------------------------------------
        // Odczytaj stan wyciszenia.
        //------------------------------------------------------

        player.muted = (message[4] == '1');

        //------------------------------------------------------
        // Poinformuj wyświetlacz o zmianie stanu Mute.
        //------------------------------------------------------

        return ChangeFlags::Mute;
    }

    //==========================================================
    // Komunikat nie został jeszcze obsłużony.
    //==========================================================

    return ChangeFlags::None;
}