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
// Zmiana źródła odtwarzania.
//
// Dla klasy String możemy bezpośrednio użyć operatora
// porównania. Nie jest już potrzebna funkcja strcmp().
//----------------------------------------------------------

if (previous.source != current.source)
    {
        changes |= ChangeFlags::Source;
    }

   //----------------------------------------------------------
   // Zmiana wykonawcy.
   //----------------------------------------------------------

   if (previous.artist != current.artist)
    {
        changes |= ChangeFlags::Artist;
    }

   //----------------------------------------------------------
   // Zmiana tytułu.
   //----------------------------------------------------------

if (previous.title != current.title)
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