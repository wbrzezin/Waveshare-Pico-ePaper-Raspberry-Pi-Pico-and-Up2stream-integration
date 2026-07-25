//==============================================================
// Projekt : UP2Stream Display
// Plik    : StateComparer.h
//
// Opis:
//
// Definicja klasy StateComparer.
//
// Klasa porównuje dwa stany odtwarzacza i zwraca zestaw
// flag określających, które elementy interfejsu uległy
// zmianie.
//
//==============================================================

#ifndef STATECOMPARER_H
#define STATECOMPARER_H

#include "ChangeFlags.h"
#include "PlayerState.h"



//==============================================================
// Klasa StateComparer
//
// Odpowiada za porównanie dwóch stanów odtwarzacza.
//
//==============================================================

class StateComparer
{
public:

    //==========================================================
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
    // Zestaw flag opisujących wykryte zmiany.
    //
    //==========================================================

    static ChangeFlags compare(const PlayerState& previous,
                               const PlayerState& current);
};

#endif