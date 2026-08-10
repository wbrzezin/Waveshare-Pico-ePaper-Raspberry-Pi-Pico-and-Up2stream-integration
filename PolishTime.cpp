//==============================================================
// Projekt : UP2Stream Display
// Plik    : PolishTime.cpp
//
// Obsługa czasu polskiego i zmiany czasu letniego/zimowego.
//==============================================================

#include "PolishTime.h"


//==============================================================
// Liczba dni w miesiącu.
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
// 0 = niedziela
// 1 = poniedziałek
// ...
// 6 = sobota
//
// Algorytm Zeller / Gregorian.
//==============================================================

static int dayOfWeek(
    int year,
    int month,
    int day)
{
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

    // Zeller:
    // 0 = sobota
    // 1 = niedziela
    // ...
    //
    // Przekształcamy do:
    // 0 = niedziela

    return (h + 6) % 7;
}


//==============================================================
// Ostatnia niedziela danego miesiąca.
//==============================================================

static int lastSunday(
    int year,
    int month)
{
    int lastDay =
        daysInMonth(year, month);

    int weekday =
        dayOfWeek(
            year,
            month,
            lastDay);

    return lastDay - weekday;
}


//==============================================================
// Sprawdzenie czasu letniego w Polsce.
//==============================================================

bool isPolishSummerTime(
    int year,
    int month,
    int day,
    int hour)
{
    //----------------------------------------------------------
    // Styczeń i luty - zawsze czas zimowy.
    //----------------------------------------------------------

    if (month < 3)
        return false;

    //----------------------------------------------------------
    // Kwiecień - wrzesień - zawsze czas letni.
    //----------------------------------------------------------

    if (month > 3 && month < 10)
        return true;

    //----------------------------------------------------------
    // Listopad i grudzień - czas zimowy.
    //----------------------------------------------------------

    if (month > 10)
        return false;

    //----------------------------------------------------------
    // Marzec.
    //
    // Zmiana następuje w ostatnią niedzielę marca.
    //----------------------------------------------------------

    if (month == 3)
    {
        int sunday =
            lastSunday(year, 3);

        if (day > sunday)
            return true;

        if (day < sunday)
            return false;

        //------------------------------------------------------
        // W dniu zmiany:
        //
        // przed 02:00 - czas zimowy
        // od 02:00   - czas letni
        //------------------------------------------------------

        return hour >= 2;
    }

    //----------------------------------------------------------
    // Październik.
    //
    // Zmiana następuje w ostatnią niedzielę października.
    //----------------------------------------------------------

    if (month == 10)
    {
        int sunday =
            lastSunday(year, 10);

        if (day < sunday)
            return true;

        if (day > sunday)
            return false;

        //------------------------------------------------------
        // W dniu zmiany:
        //
        // przed 03:00 - czas letni
        // od 03:00   - czas zimowy
        //------------------------------------------------------

        return hour < 3;
    }

    return false;
}


//==============================================================
// Dodanie jednej godziny do daty/czasu.
//==============================================================

static void addOneHour(
    PolishTime& time)
{
    time.hour++;

    if (time.hour < 24)
        return;

    time.hour = 0;

    time.day++;

    if (time.day <= daysInMonth(
                        time.year,
                        time.month))
    {
        return;
    }

    time.day = 1;
    time.month++;

    if (time.month <= 12)
        return;

    time.month = 1;
    time.year++;
}


//==============================================================
// Konwersja czasu UP2Stream -> czas polski.
//==============================================================

PolishTime convertToPolishTime(
    const Up2StreamTime& source)
{
    PolishTime result;

    if (!source.valid)
        return result;

    //----------------------------------------------------------
    // Przepisz czas źródłowy.
    //----------------------------------------------------------

    result.year = source.year;
    result.month = source.month;
    result.day = source.day;

    result.hour = source.hour;
    result.minute = source.minute;
    result.second = source.second;

    //----------------------------------------------------------
    // UP2Stream w naszym przypadku pracuje z bazowym czasem
    // odpowiadającym UTC+1.
    //
    // W okresie letnim Polska jest o godzinę do przodu.
    //----------------------------------------------------------

    if (isPolishSummerTime(
            result.year,
            result.month,
            result.day,
            result.hour))
    {
        addOneHour(result);
    }

    result.valid = true;

    return result;
}