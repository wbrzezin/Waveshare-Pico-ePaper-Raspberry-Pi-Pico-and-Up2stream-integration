//==============================================================
// Projekt : UP2Stream Display
// Plik    : StateComparer.cpp
//
// Opis:                                                        // Description:
//
// Implementacja klasy StateComparer.                          // Implementation of the StateComparer class.
//
// Klasa porównuje dwa stany odtwarzacza i zwraca zestaw        // The class compares two player states and returns a set
// flag określających, które elementy uległy zmianie.           // of flags indicating which elements have changed.
//
//==============================================================

#include "StateComparer.h"

#include <cstring>


//==============================================================
// Funkcja compare()                                           // compare() function
//
// Porównuje dwa stany odtwarzacza.                             // Compares two player states.
//
// Parametry:                                                   // Parameters:
//
// previous                                                      // previous
//      Poprzedni stan odtwarzacza.                             //      Previous player state.
//
// current                                                       // current
//      Aktualny stan odtwarzacza.                              //      Current player state.
//
// Zwraca:                                                       // Returns:
//
// Zestaw flag opisujących wszystkie wykryte zmiany.             // A set of flags describing all detected changes.
//
//==============================================================

ChangeFlags StateComparer::compare(const PlayerState& previous,
                                   const PlayerState& current)
{
    //----------------------------------------------------------
    // Brak wykrytych zmian.                                    // No changes detected.
    //----------------------------------------------------------

    ChangeFlags changes = ChangeFlags::None;


//----------------------------------------------------------//-----------------------------------------------//
// Zmiana źródła odtwarzania.                                // Playback source changed.
//                                                              //
// Dla klasy String możemy bezpośrednio użyć operatora         // For the String class, the comparison operator
// porównania. Nie jest już potrzebna funkcja strcmp().        // can be used directly. The strcmp() function is
//                                                              // no longer needed.
//----------------------------------------------------------//-----------------------------------------------//

if (previous.source != current.source)
    {
        changes |= ChangeFlags::Source;
    }

   //----------------------------------------------------------//-----------------------------------------------//
   // Zmiana wykonawcy.                                        // Artist changed.
   //----------------------------------------------------------//-----------------------------------------------//

   if (previous.artist != current.artist)
    {
        changes |= ChangeFlags::Artist;
    }

   //----------------------------------------------------------//-----------------------------------------------//
   // Zmiana tytułu.                                           // Title changed.
   //----------------------------------------------------------//-----------------------------------------------//

if (previous.title != current.title)
    {
        changes |= ChangeFlags::Title;
    }

    //----------------------------------------------------------
    // Aktualny czas odtwarzania.                              // Current playback time.
    //----------------------------------------------------------

    if (strcmp(previous.currentTime,
               current.currentTime) != 0)
    {
        changes |= ChangeFlags::CurrentTime;
    }

    //----------------------------------------------------------
    // Całkowity czas utworu.                                  // Total track duration.
    //----------------------------------------------------------

    if (strcmp(previous.totalTime,
               current.totalTime) != 0)
    {
        changes |= ChangeFlags::TotalTime;
    }

    //----------------------------------------------------------
    // Postęp odtwarzania.                                    // Playback progress.
    //----------------------------------------------------------

    if (previous.progress != current.progress)
    {
        changes |= ChangeFlags::Progress;
    }

    //----------------------------------------------------------
    // Głośność.                                               // Volume.
    //----------------------------------------------------------

    if (previous.volume != current.volume)
    {
        changes |= ChangeFlags::Volume;
    }

    //----------------------------------------------------------
    // Stan odtwarzacza.                                      // Player state.
    //----------------------------------------------------------

    if (previous.playing != current.playing)
    {
        changes |= ChangeFlags::PlayState;
    }

    //----------------------------------------------------------
    // Zwrócenie zestawu wykrytych zmian.                      // Return the set of detected changes.
    //----------------------------------------------------------

    return changes;
}