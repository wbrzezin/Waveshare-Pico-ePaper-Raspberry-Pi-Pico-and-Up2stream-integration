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

#include <stdint.h>

#include "PlayerState.h"


//==============================================================
// Typ wyliczeniowy ChangeFlags
//
// Reprezentuje zestaw zmian wykrytych pomiędzy dwoma stanami
// odtwarzacza.
//
// Flagi mogą być łączone operatorami bitowymi.
//
//==============================================================

enum class ChangeFlags : uint16_t
{
    None        = 0x0000,

    Source      = 0x0001,
    Artist      = 0x0002,
    Title       = 0x0004,

    CurrentTime = 0x0008,
    TotalTime   = 0x0010,
    Progress    = 0x0020,

    Volume      = 0x0040,

    PlayState   = 0x0080
};


//==============================================================
// Operatory bitowe dla typu ChangeFlags.
//
// Dzięki nim możliwe jest wygodne łączenie wielu flag.
//
//==============================================================

inline ChangeFlags operator|(ChangeFlags lhs,
                             ChangeFlags rhs)
{
    return static_cast<ChangeFlags>(
        static_cast<uint16_t>(lhs) |
        static_cast<uint16_t>(rhs));
}


inline ChangeFlags operator&(ChangeFlags lhs,
                             ChangeFlags rhs)
{
    return static_cast<ChangeFlags>(
        static_cast<uint16_t>(lhs) &
        static_cast<uint16_t>(rhs));
}


inline ChangeFlags& operator|=(ChangeFlags& lhs,
                               ChangeFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}


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