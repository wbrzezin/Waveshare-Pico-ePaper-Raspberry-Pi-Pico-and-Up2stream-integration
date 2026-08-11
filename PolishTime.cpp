//==============================================================
// Projekt : UP2Stream Display
// Plik    : PolishTime.cpp
//
// Opis:
//
// Implementacja obliczania czasu obowiązującego w Polsce.
//
// Up2Stream zwraca czas UTC oraz offset:
//
//     TME:2026-08-10 22:17:03 (+1);
//
// Interpretujemy go jako:
//
//     UTC    = 22:17:03
//     offset = +1
//
// W sierpniu w Polsce obowiązuje dodatkowo czas letni:
//
//     DST = +1
//
// Wynik:
//
//     22:17 + 1 + 1 = 00:17 następnego dnia
//
//==============================================================

#include "PolishTime.h"


//==============================================================
// Liczba dni w miesiącu.
//
// Uwzględniany jest rok przestępny.
//==============================================================

static int daysInMonth(
    int year,
    int month)
{
    switch (month)
    {
        case 1:
            return 31;

        case 2:
        {
            bool leap =
                ((year % 4 == 0 && year % 100 != 0) ||
                 (year % 400 == 0));

            return leap ? 29 : 28;
        }

        case 3:
            return 31;

        case 4:
            return 30;

        case 5:
            return 31;

        case 6:
            return 30;

        case 7:
            return 31;

        case 8:
            return 31;

        case 9:
            return 30;

        case 10:
            return 31;

        case 11:
            return 30;

        case 12:
            return 31;
    }

    return 30;
}


//==============================================================
// Dzień tygodnia.
//
// Wynik:
//
//     0 = niedziela
//     1 = poniedziałek
//     2 = wtorek
//     3 = środa
//     4 = czwartek
//     5 = piątek
//     6 = sobota
//
//==============================================================

static int dayOfWeek(
    int year,
    int month,
    int day)
{
    //----------------------------------------------------------
    // Algorytm Zeller'a.
//
// Styczeń i luty traktujemy jako miesiące 13 i 14
    // poprzedniego roku.
    //----------------------------------------------------------

    if (month < 3)
    {
        month += 12;
        year--;
    }

    int k = year % 100;
    int j = year / 100;

    int h =
        (day +
         (13 * (month + 1)) / 5 +
         k +
         k / 4 +
         j / 4 +
         5 * j) % 7;

    return (h + 6) % 7;
}


//==============================================================
// Numer ostatniej niedzieli miesiąca.
//==============================================================

static int lastSunday(
    int year,
    int month)
{
    int lastDay =
        daysInMonth(
            year,
            month);

    int weekday =
        dayOfWeek(
            year,
            month,
            lastDay);

    return lastDay - weekday;
}


//==============================================================
// Sprawdzenie czasu letniego w Polsce.
//
// UWAGA:
//
// Funkcja otrzymuje CZAS UTC.
//
// W Polsce:
//
//     ostatnia niedziela marca
//     01:00 UTC -> rozpoczęcie czasu letniego
//
//     ostatnia niedziela października
//     01:00 UTC -> zakończenie czasu letniego
//
// Dzięki sprawdzaniu UTC unikamy problemu z powtarzającą się
// godziną podczas jesiennej zmiany czasu.
//
//==============================================================

bool isPolishSummerTime(
    int year,
    int month,
    int day,
    int hour)
{
    //----------------------------------------------------------
    // Styczeń i luty - czas standardowy.
    //----------------------------------------------------------

    if (month < 3)
        return false;


    //----------------------------------------------------------
    // Kwiecień - wrzesień - czas letni.
    //----------------------------------------------------------

    if (month > 3 && month < 10)
        return true;


    //----------------------------------------------------------
    // Listopad i grudzień - czas standardowy.
    //----------------------------------------------------------

    if (month > 10)
        return false;


    //----------------------------------------------------------
    // MARZEC
    //----------------------------------------------------------

    if (month == 3)
    {
        int sunday =
            lastSunday(
                year,
                3);

        if (day < sunday)
            return false;

        if (day > sunday)
            return true;

        //------------------------------------------------------
        // W dniu zmiany:
        //
        // przed 01:00 UTC -> czas standardowy
        // od 01:00 UTC   -> czas letni
        //------------------------------------------------------

        return hour >= 1;
    }


    //----------------------------------------------------------
    // PAŹDZIERNIK
    //----------------------------------------------------------

    if (month == 10)
    {
        int sunday =
            lastSunday(
                year,
                10);

        if (day < sunday)
            return true;

        if (day > sunday)
            return false;

        //------------------------------------------------------
        // W dniu zmiany:
        //
        // przed 01:00 UTC -> czas letni
        // od 01:00 UTC   -> czas standardowy
        //------------------------------------------------------

        return hour < 1;
    }


    return false;
}


//==============================================================
// Dodanie godzin do czasu.
//
// Obsługujemy zarówno wartości dodatnie, jak i ujemne.
//
// Dzięki temu algorytm działa również wtedy, gdy Up2Stream
// zwróci ujemny offset.
//
//==============================================================

static void addHours(
    PolishTime& time,
    int hours)
{
    //----------------------------------------------------------
    // Dodawanie godzin dodatnich.
    //----------------------------------------------------------

    while (hours > 0)
    {
        time.hour++;

        if (time.hour < 24)
        {
            hours--;
            continue;
        }

        //------------------------------------------------------
        // Przejście przez północ.
        //------------------------------------------------------

        time.hour = 0;
        time.day++;

        //------------------------------------------------------
        // Przejście do następnego miesiąca.
        //------------------------------------------------------

        if (time.day >
            daysInMonth(
                time.year,
                time.month))
        {
            time.day = 1;
            time.month++;

            //--------------------------------------------------
            // Przejście do następnego roku.
            //--------------------------------------------------

            if (time.month > 12)
            {
                time.month = 1;
                time.year++;
            }
        }

        hours--;
    }


    //----------------------------------------------------------
    // Dodawanie godzin ujemnych.
    //----------------------------------------------------------

    while (hours < 0)
    {
        time.hour--;

        if (time.hour >= 0)
        {
            hours++;
            continue;
        }

        //------------------------------------------------------
        // Przejście przez północ wstecz.
        //------------------------------------------------------

        time.hour = 23;
        time.day--;

        //------------------------------------------------------
        // Przejście do poprzedniego miesiąca.
        //------------------------------------------------------

        if (time.day < 1)
        {
            time.month--;

            //--------------------------------------------------
            // Przejście do poprzedniego roku.
            //--------------------------------------------------

            if (time.month < 1)
            {
                time.month = 12;
                time.year--;
            }

            time.day =
                daysInMonth(
                    time.year,
                    time.month);
        }

        hours++;
    }
}


//==============================================================
// Konwersja czasu Up2Stream na czas polski.
//
// Schemat:
//
//     UTC
//       +
//     offset z TME
//       +
//     DST (+1, jeżeli obowiązuje)
//       =
//     czas polski
//
//==============================================================

PolishTime convertToPolishTime(
    const Up2StreamTime& source)
{
    PolishTime result;


    //----------------------------------------------------------
    // Niepoprawny czas źródłowy.
    //----------------------------------------------------------

    if (!source.valid)
        return result;


    //----------------------------------------------------------
    // Skopiowanie czasu UTC.
    //----------------------------------------------------------

    result.year = source.year;
    result.month = source.month;
    result.day = source.day;

    result.hour = source.hour;
    result.minute = source.minute;
    result.second = source.second;


    //----------------------------------------------------------
    // Ustalenie, czy dla otrzymanej daty UTC obowiązuje
    // w Polsce czas letni.
    //----------------------------------------------------------

    bool summerTime =
        isPolishSummerTime(
            source.year,
            source.month,
            source.day,
            source.hour);


    //----------------------------------------------------------
    // Dodanie offsetu przekazanego przez Up2Stream.
    //
    // Przykład:
    //
    //     22:17 UTC
    //     +1
    //     = 23:17
    //----------------------------------------------------------

    addHours(
        result,
        source.utcOffset);


    //----------------------------------------------------------
    // Jeżeli obowiązuje czas letni, dodajemy dodatkową godzinę.
    //
    // Przykład:
    //
    //     23:17 + 1h DST
    //     = 00:17 następnego dnia
    //----------------------------------------------------------

    if (summerTime)
    {
        addHours(
            result,
            1);
    }


    //----------------------------------------------------------
    // Wynik jest poprawny.
    //----------------------------------------------------------

    result.valid = true;

    return result;
}