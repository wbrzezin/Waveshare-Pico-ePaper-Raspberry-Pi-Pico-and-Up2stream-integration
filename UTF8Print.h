/******************************************************************************
 * @file    PrintPL.h
 * @brief   Obsługa polskich znaków dla klasy Print.
 *
 * Moduł umożliwia wyświetlanie tekstów UTF-8 z polskimi znakami
 * na wyświetlaczach wykorzystujących zmodyfikowane fonty Adafruit GFX.
 *
 * Autor: Waldemar Brzeziński
 ******************************************************************************/

#ifndef PRINTPL_H
#define PRINTPL_H

#include <Print.h>

/**
 * @brief Wyświetla tekst UTF-8 z obsługą polskich znaków.
 *
 * Funkcja analizuje tekst zapisany w UTF-8 i zamienia polskie litery
 * na wewnętrzne kody znaków (0x80...0x91), zgodne ze zmodyfikowanym
 * fontem wygenerowanym przez fontconvert.
 *
 * @param out  Obiekt klasy Print (np. wyświetlacz, Serial).
 * @param text Tekst zakończony znakiem '\0'.
 */
void printPL(Print &out, const char *text);

#endif