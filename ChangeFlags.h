//==============================================================
// Projekt : UP2Stream Display
// Plik    : ChangeFlags.h
//
// Opis:
//
// Definicja typu ChangeFlags.
//
// Typ opisuje elementy interfejsu użytkownika, które uległy
// zmianie i wymagają odświeżenia.
//
//==============================================================

#ifndef CHANGEFLAGS_H
#define CHANGEFLAGS_H

#include <stdint.h>

//==============================================================
// Typ wyliczeniowy ChangeFlags
//
// Poszczególne wartości mogą być łączone operatorami bitowymi.
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

    PlayState   = 0x0080,

    Mute        = 0x0100,
};


//==============================================================
// Operatory bitowe
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

#endif