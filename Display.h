
//==============================================================
// Projekt : UP2Stream Display
// Plik    : Display.h
// Autor   : Waldemar Brzeziński
// Opis    : Deklaracja klasy Display.
//==============================================================


#ifndef DISPLAY_H
#define DISPLAY_H

//--------------------------------------------------------------
// Klasa Display
//
// Odpowiada za obsługę wyświetlacza e-paper oraz rysowanie
// interfejsu użytkownika odtwarzacza UP2Stream.
//
// Zadania klasy:
// - inicjalizacja wyświetlacza,
// - wyświetlenie ekranu startowego,
// - rysowanie głównego ekranu odtwarzacza,
// - aktualizacja elementów dynamicznych,
// - obsługa przewijania długich nazw utworów i wykonawców.
//--------------------------------------------------------------
class Display
{
public:

    //==========================================================
    // Funkcje publiczne
    // Wywoływane przez główny program.
    //==========================================================

    //----------------------------------------------------------
    // Inicjalizacja wyświetlacza.
    //
    // Funkcja konfiguruje interfejs SPI, uruchamia sterownik
    // wyświetlacza oraz wyświetla ekran startowy.
    //
    // Zwraca:
    // true - inicjalizacja zakończyła się powodzeniem.
    //----------------------------------------------------------
    bool begin();

    //----------------------------------------------------------
    // Wyświetlenie ekranu startowego.
    //
    // Funkcja wykonywana jednorazowo podczas uruchamiania
    // urządzenia.
    //----------------------------------------------------------
    void splash();

    //----------------------------------------------------------
    // Wyświetlenie głównego ekranu odtwarzacza.
    //
    // Parametry:
    // source - źródło dźwięku (Spotify, AirPlay, DLNA...)
    // artist - nazwa wykonawcy
    // title  - tytuł aktualnie odtwarzanego utworu
    //----------------------------------------------------------
    void showPlayer(const char* source,
                    const char* artist,
                    const char* title);

    //----------------------------------------------------------
    // Aktualizacja elementów dynamicznych.
    //
    // Docelowo funkcja będzie odpowiedzialna za płynne
    // przewijanie długich nazw utworów i wykonawców oraz
    // odświeżanie wybranych fragmentów ekranu.
    //----------------------------------------------------------
    void scrollText();


private:

    //==========================================================
    // Funkcje rysujące statyczne elementy interfejsu
    //==========================================================

    //----------------------------------------------------------
    // Rysowanie nagłówka z nazwą źródła dźwięku.
    //----------------------------------------------------------
    void drawHeader(const char* source);

    //----------------------------------------------------------
    // Rysowanie tytułu aktualnie odtwarzanego utworu.
    //----------------------------------------------------------
    void drawTitle(const char* title);

    //----------------------------------------------------------
    // Rysowanie nazwy wykonawcy.
    //----------------------------------------------------------
    void drawArtist(const char* artist);

    //----------------------------------------------------------
    // Rysowanie poziomej linii oddzielającej sekcje ekranu.
    //----------------------------------------------------------
    void drawSeparator(int y);

    //----------------------------------------------------------
    // Rysowanie paska postępu odtwarzania.
    //
    // currentTime - aktualny czas odtwarzania
    // totalTime   - całkowity czas utworu
    // progress    - postęp odtwarzania (0...100%)
    //----------------------------------------------------------
    void drawPlaybackBar(const char* currentTime,
                         const char* totalTime,
                         int progress);

    //==========================================================
    // Dane aktualnie wyświetlane na ekranie
    //==========================================================

    //----------------------------------------------------------
    // Aktualnie wyświetlany tytuł i wykonawca.
    //
    // Dane są zapamiętywane, aby możliwe było ich późniejsze
    // odświeżanie bez konieczności ponownego wywoływania
    // funkcji showPlayer().
    //----------------------------------------------------------
    const char* currentTitle = nullptr;
    const char* currentArtist = nullptr;

    //==========================================================
    // Parametry przewijania tekstu
    //==========================================================

    //----------------------------------------------------------
    // Aktualne przesunięcie tekstu w pikselach.
    //
    // Wartości będą wykorzystywane podczas płynnego przewijania
    // długich nazw utworów i wykonawców.
    //----------------------------------------------------------
    int titleOffset = 0;
    int artistOffset = 0;
};

#endif