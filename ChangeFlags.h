//==============================================================//-----------------------------------------------//
// Projekt : UP2Stream Display                              // Project : UP2Stream Display                   //
// Plik    : ChangeFlags.h                                  // File    : ChangeFlags.h                       //
//                                                          //                                               //
// Opis:                                                    // Description:                                  //
//                                                          //                                               //
// Definicja typu ChangeFlags.                              // Definition of the ChangeFlags type.           //
//                                                          //                                               //
// Typ opisuje elementy stanu odtwarzacza, które uległy     // The type describes player state elements that  //
// zmianie i mogą wymagać odświeżenia interfejsu             // have changed and may require a user interface //
// użytkownika lub reakcji innego modułu.                   // update or a response from another module.     //
//==============================================================//-----------------------------------------------//


#ifndef CHANGEFLAGS_H
#define CHANGEFLAGS_H


#include <stdint.h>


//==============================================================//-----------------------------------------------//
// Typ wyliczeniowy ChangeFlags                              // ChangeFlags enumeration                       //
//                                                          //                                               //
// Poszczególne wartości reprezentują pojedyncze bity       // Each value represents a single bit and can be //
// i mogą być łączone operatorami bitowymi.                  // combined using bitwise operators.             //
//                                                          //                                               //
// Dzięki temu jedna wartość ChangeFlags może informować    // This allows a single ChangeFlags value to     //
// jednocześnie o wielu zmianach stanu.                     // indicate multiple state changes at once.     //
//==============================================================//-----------------------------------------------//

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

    Vendor      = 0x0200,

    Standby     = 0x0400
};


//==============================================================//-----------------------------------------------//
// Operatory bitowe                                          // Bitwise operators                            //
//==============================================================//-----------------------------------------------//

//--------------------------------------------------------------//-----------------------------------------------//
// Operator OR.                                               // OR operator.                                 //
//                                                              //                                               //
// Umożliwia połączenie kilku flag w jedną wartość.             // Allows multiple flags to be combined into    //
//                                                              // a single value.                              //
//                                                              //                                               //
// Przykład:                                                   // Example:                                     //
//                                                              //                                               //
// flags = ChangeFlags::Title | ChangeFlags::Artist;            // flags = ChangeFlags::Title | ChangeFlags::Artist;
//--------------------------------------------------------------//-----------------------------------------------//

inline ChangeFlags operator|(ChangeFlags lhs,
                             ChangeFlags rhs)
{
    return static_cast<ChangeFlags>(
        static_cast<uint16_t>(lhs) |
        static_cast<uint16_t>(rhs));
}


//--------------------------------------------------------------//-----------------------------------------------//
// Operator AND.                                              // AND operator.                                //
//                                                              //                                               //
// Umożliwia sprawdzenie, czy określona flaga jest ustawiona.   // Allows checking whether a specific flag is    //
//                                                              // set.                                          //
//                                                              //                                               //
// Przykład:                                                   // Example:                                     //
//                                                              //                                               //
// if ((flags & ChangeFlags::Title) != ChangeFlags::None)       // if ((flags & ChangeFlags::Title) != ChangeFlags::None)
//--------------------------------------------------------------//-----------------------------------------------//

inline ChangeFlags operator&(ChangeFlags lhs,
                             ChangeFlags rhs)
{
    return static_cast<ChangeFlags>(
        static_cast<uint16_t>(lhs) &
        static_cast<uint16_t>(rhs));
}


//--------------------------------------------------------------//-----------------------------------------------//
// Operator OR z przypisaniem.                                // Compound OR assignment operator.              //
//                                                              //                                               //
// Dodaje określoną flagę do istniejącego zestawu flag.          // Adds a specified flag to an existing set of   //
//                                                              // flags.                                        //
//--------------------------------------------------------------//-----------------------------------------------//

inline ChangeFlags& operator|=(ChangeFlags& lhs,
                               ChangeFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}


#endif