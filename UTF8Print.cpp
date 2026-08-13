/******************************************************************************
 * @file    UTF8Print.cpp
 * @brief   Obsługa polskich znaków dla klasy Print.            // Polish character support for the Print class.
 ******************************************************************************/

#include "UTF8Print.h"


/**
 * @brief Zamienia kod Unicode na kod znaku używany przez font. // Converts a Unicode code point to the character code
 *                                                              // used by the font.
 *
 * Dla znaków ASCII zwracany jest ten sam kod.                  // For ASCII characters, the same code is returned.
 * Obsługa polskich znaków zostanie dodana w kolejnych krokach. // Polish character support will be added in subsequent
 *                                                              // steps.
 *
 * @param unicode Kod Unicode znaku.                           // Unicode code point of the character.
 * @return Kod znaku zgodny z wygenerowanym fontem.             // Character code compatible with the generated font.
 */
static uint8_t mapUnicode(uint16_t unicode)
{
    /* Znaki ASCII są zgodne z kodowaniem czcionki. */           // ASCII characters are compatible with the font encoding.
    if (unicode < 0x80)
        return (uint8_t)unicode;

    /* Polskie znaki diakrytyczne. */                           // Polish diacritical characters.
    switch (unicode)
    {
        case 0x0104: return 0x80;   // Ą
        case 0x0106: return 0x81;   // Ć
        case 0x0118: return 0x82;   // Ę
        case 0x0141: return 0x83;   // Ł
        case 0x0143: return 0x84;   // Ń
        case 0x00D3: return 0x85;   // Ó
        case 0x015A: return 0x86;   // Ś
        case 0x0179: return 0x87;   // Ź
        case 0x017B: return 0x88;   // Ż

        case 0x0105: return 0x89;   // ą
        case 0x0107: return 0x8A;   // ć
        case 0x0119: return 0x8B;   // ę
        case 0x0142: return 0x8C;   // ł
        case 0x0144: return 0x8D;   // ń
        case 0x00F3: return 0x8E;   // ó
        case 0x015B: return 0x8F;   // ś
        case 0x017A: return 0x90;   // ź
        case 0x017C: return 0x91;   // ż
    }

    /* Nieznany znak. */                                        // Unknown character.
    return '?';
}


/**
 * @brief Wyświetla tekst UTF-8.                                // Displays UTF-8 text.
 *
 * Na obecnym etapie:                                          // At the current stage:
 * - obsługuje znaki ASCII,                                     // - supports ASCII characters,
 * - rozpoznaje początek dwubajtowej sekwencji UTF-8,          // - recognizes the beginning of a two-byte UTF-8 sequence,
 * - nie dekoduje jeszcze znaków UTF-8.                         // - does not yet decode UTF-8 characters.
 */
void printPL(Print &out, const char *text)
{
    while (*text)
    {
        uint8_t c = (uint8_t)*text++;
        uint16_t unicode = c;

        //------------------------------------------------------//-----------------------------------------------//
        // Wewnętrzne kody znaków specjalnych.                 // Internal codes for special characters.
        //
        // Zakres 0x80...0x96 wykorzystywany jest przez         // The 0x80...0x96 range is used by the generated
        // wygenerowaną czcionkę do przechowywania polskich    // font to store Polish characters and user interface
        // znaków oraz ikon interfejsu użytkownika.             // icons.
        //------------------------------------------------------//-----------------------------------------------//

        if (c >= 0x80 && c <= 0x96)
        {
            out.write(c);
            continue;
        }

        /* Znaki ASCII. */                                      // ASCII characters.
        if (c < 0x80)
        {
            out.write(mapUnicode(unicode));
        }

        /* Początek dwubajtowej sekwencji UTF-8. */              // Beginning of a two-byte UTF-8 sequence.
        else if ((c & 0xE0) == 0xC0)
        {
            /* Pobierz drugi bajt sekwencji UTF-8. */            // Get the second byte of the UTF-8 sequence.
            uint8_t c2 = (uint8_t)*text;

            /* Sprawdź poprawność drugiego bajtu. */             // Check the validity of the second byte.
            if ((c2 & 0xC0) == 0x80)
            {
                text++;

                /* Złóż kod Unicode z dwóch bajtów UTF-8. */    // Assemble the Unicode code point from the two UTF-8 bytes.
                unicode = ((uint16_t)(c & 0x1F) << 6) |
                          (uint16_t)(c2 & 0x3F);

                out.write(mapUnicode(unicode));
            }
            else
            {
                out.write('!');
            }
        }

        /* Nieobsługiwany bajt UTF-8. */                        // Unsupported UTF-8 byte.
        else
        {
            out.write('?');
        }
    }
}