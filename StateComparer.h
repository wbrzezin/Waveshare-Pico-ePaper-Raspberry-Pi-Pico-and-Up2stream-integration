//==============================================================
// Projekt : UP2Stream Display
// Plik    : StateComparer.h
//
// Opis:                                                        // Description:
//
// Definicja klasy StateComparer.                              // Definition of the StateComparer class.
//
// Klasa porównuje dwa stany odtwarzacza i zwraca zestaw        // The class compares two player states and returns a set
// flag określających, które elementy interfejsu uległy         // of flags indicating which interface elements have
// zmianie.                                                     // changed.
//
//==============================================================

#ifndef STATECOMPARER_H
#define STATECOMPARER_H

#include "ChangeFlags.h"
#include "PlayerState.h"



//==============================================================
// Klasa StateComparer                                         // StateComparer class
//
// Odpowiada za porównanie dwóch stanów odtwarzacza.            // Responsible for comparing two player states.
//
//==============================================================

class StateComparer
{
public:

    //==========================================================
    // Funkcja compare()                                       // compare() function
    //
    // Porównuje dwa stany odtwarzacza.                        // Compares two player states.
    //
    // Parametry:                                              // Parameters:
    //
    // previous                                                 // previous
    //      Poprzedni stan odtwarzacza.                         //      Previous player state.
    //
    // current                                                  // current
    //      Aktualny stan odtwarzacza.                          //      Current player state.
    //
    // Zwraca:                                                  // Returns:
    //
    // Zestaw flag opisujących wykryte zmiany.                  // A set of flags describing the detected changes.
    //
    //==========================================================

    static ChangeFlags compare(const PlayerState& previous,
                               const PlayerState& current);
};

#endif