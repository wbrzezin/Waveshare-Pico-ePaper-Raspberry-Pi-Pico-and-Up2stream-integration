//==============================================================
// Projekt : UP2Stream Display
// Plik    : HardwareConfig.h
//
// Opis:
//
// Centralna konfiguracja sprzętowa projektu.
//
// W pliku znajdują się definicje wszystkich wykorzystywanych
// pinów mikrokontrolera Raspberry Pi Pico.
//
// Dzięki temu zmiana płytki lub sposobu podłączenia wymaga
// modyfikacji tylko jednego pliku.
//==============================================================

#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

#include <Arduino.h>

//==============================================================
// Wyświetlacz e-paper
//==============================================================

constexpr uint8_t DISPLAY_CS_PIN   = 9;
constexpr uint8_t DISPLAY_DC_PIN   = 8;
constexpr uint8_t DISPLAY_RST_PIN  = 12;
constexpr uint8_t DISPLAY_BUSY_PIN = 13;


//==============================================================
// Komunikacja UART z modułem Up2Stream
//==============================================================

constexpr uint8_t UP2STREAM_UART_TX_PIN = 4;
constexpr uint8_t UP2STREAM_UART_RX_PIN = 5;


//--------------------------------------------------------------
// Port UART wykorzystywany do komunikacji z modułem Up2Stream.
//
// W rdzeniu Raspberry Pi Pico/RP2040 by Earle Philhower:
//
//   Serial1 -> UART0
//   Serial2 -> UART1
//
// Up2Stream wykorzystuje UART1.
//--------------------------------------------------------------

#define UP2STREAM_SERIAL Serial2

#define UP2STREAM_SERIAL Serial2

#endif