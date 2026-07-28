/******************************************************************************
 * @file    UTF8Print.cpp
 * @brief   Obsługa polskich znaków dla klasy Print.
 ******************************************************************************/

#include "UTF8Print.h"


/**
 * @brief Zamienia kod Unicode na kod znaku używany przez font.
 *
 * Dla znaków ASCII zwracany jest ten sam kod.
 * Obsługa polskich znaków zostanie dodana w kolejnych krokach.
 *
 * @param unicode Kod Unicode znaku.
 * @return Kod znaku zgodny z wygenerowanym fontem.
 */
static uint8_t mapUnicode(uint16_t unicode)
{
    /* Znaki ASCII są zgodne z kodowaniem czcionki. */
    if (unicode < 0x80)
        return (uint8_t)unicode;

    /* Polskie znaki diakrytyczne. */
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

    /* Nieznany znak. */
    return '?';
}
/**
 * @brief Wyświetla tekst UTF-8.
 *
 * Na obecnym etapie:
 * - obsługuje znaki ASCII,
 * - rozpoznaje początek dwubajtowej sekwencji UTF-8,
 * - nie dekoduje jeszcze znaków UTF-8.
 */
void printPL(Print &out, const char *text)
{
    while (*text)
    {
        uint8_t c = (uint8_t)*text++;
        uint16_t unicode = c;

        /* Znaki ASCII. */
        if (c < 0x80)
        {
            out.write(mapUnicode(unicode));
        }
        /* Początek dwubajtowej sekwencji UTF-8. */
       
        else if ((c & 0xE0) == 0xC0)
{
    /* Pobierz drugi bajt sekwencji UTF-8. */
    uint8_t c2 = (uint8_t)*text;

    /* Sprawdź poprawność drugiego bajtu. */
    if ((c2 & 0xC0) == 0x80)
    {
        text++;

        /* Złóż kod Unicode z dwóch bajtów UTF-8. */
        unicode = ((uint16_t)(c & 0x1F) << 6) |
                  (uint16_t)(c2 & 0x3F);

        out.write(mapUnicode(unicode));          
    }
    else
    {
        out.write('!');
    }
}

        /* Nieobsługiwany bajt UTF-8. */
        else
        {
            out.write('?');
        }
    }
}