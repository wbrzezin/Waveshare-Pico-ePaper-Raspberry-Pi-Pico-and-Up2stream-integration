//==============================================================
// Projekt : UP2Stream Display
// Plik    : StateComparer.cpp
//
// Opis:
//
// Implementacja klasy StateComparer.
//
// Klasa porównuje dwa stany odtwarzacza i zwraca zestaw
// flag określających, które elementy uległy zmianie.
//
//==============================================================

#include "StateComparer.h"

#include <cstring>


//==============================================================
// Funkcja compare()
//
// Porównuje dwa stany odtwarzacza.
//
// Parametry:
//
// previous
//      Poprzedni stan odtwarzacza.
//
// current
//      Aktualny stan odtwarzacza.
//
// Zwraca:
//
// Zestaw flag opisujących wszystkie wykryte zmiany.
//
//==============================================================

ChangeFlags StateComparer::compare(const PlayerState& previous,
                                   const PlayerState& current)
{
    //----------------------------------------------------------
    // Brak wykrytych zmian.
    //----------------------------------------------------------

    ChangeFlags changes = ChangeFlags::None;

    //----------------------------------------------------------
    // Źródło dźwięku.
    //----------------------------------------------------------

    if (strcmp(previous.source, current.source) != 0)
    {
        changes |= ChangeFlags::Source;
    }

    //----------------------------------------------------------
    // Wykonawca.
    //----------------------------------------------------------

    if (strcmp(previous.artist, current.artist) != 0)
    {
        changes |= ChangeFlags::Artist;
    }

    //----------------------------------------------------------
    // Tytuł utworu.
    //----------------------------------------------------------

    if (strcmp(previous.title, current.title) != 0)
    {
        changes |= ChangeFlags::Title;
    }

    //----------------------------------------------------------
    // Aktualny czas odtwarzania.
    //----------------------------------------------------------

    if (strcmp(previous.currentTime,
               current.currentTime) != 0)
    {
        changes |= ChangeFlags::CurrentTime;
    }

    //----------------------------------------------------------
    // Całkowity czas utworu.
    //----------------------------------------------------------

    if (strcmp(previous.totalTime,
               current.totalTime) != 0)
    {
        changes |= ChangeFlags::TotalTime;
    }

    //----------------------------------------------------------
    // Postęp odtwarzania.
    //----------------------------------------------------------

    if (previous.progress != current.progress)
    {
        changes |= ChangeFlags::Progress;
    }

    //----------------------------------------------------------
    // Głośność.
    //----------------------------------------------------------

    if (previous.volume != current.volume)
    {
        changes |= ChangeFlags::Volume;
    }

    //----------------------------------------------------------
    // Stan odtwarzacza.
    //----------------------------------------------------------

    if (previous.playing != current.playing)
    {
        changes |= ChangeFlags::PlayState;
    }

    //----------------------------------------------------------
    // Zwrócenie zestawu wykrytych zmian.
    //----------------------------------------------------------

    return changes;
}