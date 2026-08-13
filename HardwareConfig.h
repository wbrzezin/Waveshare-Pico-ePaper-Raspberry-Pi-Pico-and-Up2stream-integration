//==============================================================
// Projekt : UP2Stream Display
// Plik    : HardwareConfig.h
//
// Opis:                                                        // Description:
// Centralna konfiguracja sprzętowa projektu.                   // Central hardware configuration of the project.
//
// W pliku znajdują się definicje wszystkich wykorzystywanych   // This file contains definitions of all
// pinów mikrokontrolera Raspberry Pi Pico.                     // Raspberry Pi Pico microcontroller pins used.
//
// Dzięki temu zmiana płytki lub sposobu podłączenia wymaga     // This makes changing the board or wiring require
// modyfikacji tylko jednego pliku.                             // modifications to only one file.
//==============================================================


#ifndef HARDWARECONFIG_H


#include <Arduino.h>


//==============================================================
// Wyświetlacz e-paper                                         // E-paper display
//==============================================================


constexpr uint8_t DISPLAY_CS_PIN   = 9;
constexpr uint8_t DISPLAY_DC_PIN   = 8;
constexpr uint8_t DISPLAY_RST_PIN  = 12;
constexpr uint8_t DISPLAY_BUSY_PIN = 13;



//==============================================================
// Komunikacja UART z modułem Up2Stream                         // UART communication with the Up2Stream module
//==============================================================


constexpr uint8_t UP2STREAM_UART_TX_PIN = 4;
constexpr uint8_t UP2STREAM_UART_RX_PIN = 5;



//--------------------------------------------------------------//-----------------------------------------------//
// Port UART wykorzystywany do komunikacji z modułem Up2Stream. // UART port used for communication with the Up2Stream
//                                                              // module.
//                                                              //
// W rdzeniu Raspberry Pi Pico/RP2040 by Earle Philhower:       // In the Raspberry Pi Pico/RP2040 core by Earle Philhower:
//
//     Serial1 -> UART0                                          //     Serial1 -> UART0
//     Serial2 -> UART1                                          //     Serial2 -> UART1
//
// Up2Stream wykorzystuje UART1.                                 // Up2Stream uses UART1.
//--------------------------------------------------------------//-----------------------------------------------//


#define UP2STREAM_SERIAL Serial2




#endif