//==============================================================
// Projekt : UP2Stream Display
// Plik    : PolishTime.cpp
//
// Opis:                                                        // Description:
//
// Implementacja obliczania czasu obowiązującego w Polsce.     // Implementation of calculating the time applicable in Poland.
//
// Up2Stream zwraca czas UTC oraz offset:                       // Up2Stream returns UTC time and an offset:
//
//     TME:2026-08-10 22:17:03 (+1);                            //     TME:2026-08-10 22:17:03 (+1);
//
// Interpretujemy go jako:                                     // It is interpreted as:
//
//     UTC    = 22:17:03                                       //     UTC    = 22:17:03
//     offset = +1                                              //     offset = +1
//
// W sierpniu w Polsce obowiązuje dodatkowo czas letni:        // In August, daylight saving time is also in effect
//                                                              // in Poland:
//
//     DST = +1                                                 //     DST = +1
//
// Wynik:                                                       // Result:
//
//     22:17 + 1 + 1 = 00:17 następnego dnia                    //     22:17 + 1 + 1 = 00:17 on the following day
//
//==============================================================

#include "PolishTime.h"


//==============================================================
// Liczba dni w miesiącu.                                      // Number of days in a month.
//
// Uwzględniany jest rok przestępny.                           // Leap years are taken into account.
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
// Dzień tygodnia.                                             // Day of the week.
//
// Wynik:                                                       // Result:
//
//     0 = niedziela                                            //     0 = Sunday
//     1 = poniedziałek                                         //     1 = Monday
//     2 = wtorek                                               //     2 = Tuesday
//     3 = środa                                                //     3 = Wednesday
//     4 = czwartek                                             //     4 = Thursday
//     5 = piątek                                               //     5 = Friday
//     6 = sobota                                               //     6 = Saturday
//
//==============================================================

static int dayOfWeek(
    int year,
    int month,
    int day)
{
    //----------------------------------------------------------
    // Algorytm Zeller'a.                                       // Zeller's algorithm.
    //
    // Styczeń i luty traktujemy jako miesiące 13 i 14         // January and February are treated as months 13 and 14
    // poprzedniego roku.                                       // of the previous year.
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
// Numer ostatniej niedzieli miesiąca.                         // Day number of the last Sunday of the month.
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
// Sprawdzenie czasu letniego w Polsce.                        // Check whether daylight saving time is in effect in Poland.
//
// UWAGA:                                                       // NOTE:
//
// Funkcja otrzymuje CZAS UTC.                                  // The function receives UTC TIME.
//
// W Polsce:                                                    // In Poland:
//
//     ostatnia niedziela marca                                 //     last Sunday of March
//     01:00 UTC -> rozpoczęcie czasu letniego                  //     01:00 UTC -> start of daylight saving time
//
//     ostatnia niedziela października                           //     last Sunday of October
//     01:00 UTC -> zakończenie czasu letniego                  //     01:00 UTC -> end of daylight saving time
//
// Dzięki sprawdzaniu UTC unikamy problemu z powtarzającą się  // Checking UTC avoids the problem of the repeated
// godziną podczas jesiennej zmiany czasu.                      // hour during the autumn time change.
//
//==============================================================

bool isPolishSummerTime(
    int year,
    int month,
    int day,
    int hour)
{
    //----------------------------------------------------------
    // Styczeń i luty - czas standardowy.                       // January and February - standard time.
    //----------------------------------------------------------

    if (month < 3)
        return false;


    //----------------------------------------------------------
    // Kwiecień - wrzesień - czas letni.                       // April - September - daylight saving time.
    //----------------------------------------------------------

    if (month > 3 && month < 10)
        return true;


    //----------------------------------------------------------
    // Listopad i grudzień - czas standardowy.                  // November and December - standard time.
    //----------------------------------------------------------

    if (month > 10)
        return false;


    //----------------------------------------------------------
    // MARZEC                                                    // MARCH
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
        // W dniu zmiany:                                        // On the changeover day:
        //
        // przed 01:00 UTC -> czas standardowy                   // before 01:00 UTC -> standard time
        // od 01:00 UTC   -> czas letni                          // from 01:00 UTC -> daylight saving time
        //------------------------------------------------------

        return hour >= 1;
    }


    //----------------------------------------------------------
    // PAŹDZIERNIK                                               // OCTOBER
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
        // W dniu zmiany:                                        // On the changeover day:
        //
        // przed 01:00 UTC -> czas letni                        // before 01:00 UTC -> daylight saving time
        // od 01:00 UTC   -> czas standardowy                    // from 01:00 UTC -> standard time
        //------------------------------------------------------

        return hour < 1;
    }


    return false;
}


//==============================================================
// Dodanie godzin do czasu.                                   // Add hours to a time.
//
// Obsługujemy zarówno wartości dodatnie, jak i ujemne.        // Both positive and negative values are supported.
//
// Dzięki temu algorytm działa również wtedy, gdy Up2Stream    // This also allows the algorithm to work when Up2Stream
// zwróci ujemny offset.                                       // returns a negative offset.
//
//==============================================================

static void addHours(
    PolishTime& time,
    int hours)
{
    //----------------------------------------------------------
    // Dodawanie godzin dodatnich.                              // Adding positive hours.
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
        // Przejście przez północ.                              // Crossing midnight.
        //------------------------------------------------------

        time.hour = 0;
        time.day++;

        //------------------------------------------------------
        // Przejście do następnego miesiąca.                    // Moving to the next month.
        //------------------------------------------------------

        if (time.day >
            daysInMonth(
                time.year,
                time.month))
        {
            time.day = 1;
            time.month++;

            //--------------------------------------------------
            // Przejście do następnego roku.                    // Moving to the next year.
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
    // Dodawanie godzin ujemnych.                               // Adding negative hours.
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
        // Przejście przez północ wstecz.                       // Crossing midnight backwards.
        //------------------------------------------------------

        time.hour = 23;
        time.day--;

        //------------------------------------------------------
        // Przejście do poprzedniego miesiąca.                  // Moving to the previous month.
        //------------------------------------------------------

        if (time.day < 1)
        {
            time.month--;

            //--------------------------------------------------
            // Przejście do poprzedniego roku.                  // Moving to the previous year.
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
// Konwersja czasu Up2Stream na czas polski.                    // Convert Up2Stream time to Polish time.
//
// Schemat:                                                      // Scheme:
//
//     UTC                                                       //     UTC
//       +                                                         //       +
//     offset z TME                                              //     offset from TME
//       +                                                         //       +
//     DST (+1, jeżeli obowiązuje)                               //     DST (+1, if applicable)
//       =                                                         //       =
//     czas polski                                               //     Polish time
//
//==============================================================

PolishTime convertToPolishTime(
    const Up2StreamTime& source)
{
    PolishTime result;


    //----------------------------------------------------------
    // Niepoprawny czas źródłowy.                               // Invalid source time.
    //----------------------------------------------------------

    if (!source.valid)
        return result;


    //----------------------------------------------------------
    // Skopiowanie czasu UTC.                                   // Copy the UTC time.
    //----------------------------------------------------------

    result.year = source.year;
    result.month = source.month;
    result.day = source.day;

    result.hour = source.hour;
    result.minute = source.minute;
    result.second = source.second;


    //----------------------------------------------------------
    // Ustalenie, czy dla otrzymanej daty UTC obowiązuje        // Determine whether daylight saving time is in effect
    // w Polsce czas letni.                                     // in Poland for the received UTC date.
    //----------------------------------------------------------

    bool summerTime =
        isPolishSummerTime(
            source.year,
            source.month,
            source.day,
            source.hour);


    //----------------------------------------------------------
    // Dodanie offsetu przekazanego przez Up2Stream.             // Add the offset provided by Up2Stream.
    //
    // Przykład:                                                 // Example:
    //
    //     22:17 UTC                                             //     22:17 UTC
    //     +1                                                     //     +1
    //     = 23:17                                                //     = 23:17
    //----------------------------------------------------------

    addHours(
        result,
        source.utcOffset);


    //----------------------------------------------------------
    // Jeżeli obowiązuje czas letni, dodajemy dodatkową godzinę. // If daylight saving time is in effect, add one hour.
    //
    // Przykład:                                                 // Example:
    //
    //     23:17 + 1h DST                                        //     23:17 + 1h DST
    //     = 00:17 następnego dnia                               //     = 00:17 on the following day
    //----------------------------------------------------------

    if (summerTime)
    {
        addHours(
            result,
            1);
    }


    //----------------------------------------------------------
    // Wynik jest poprawny.                                     // The result is valid.
    //----------------------------------------------------------

    result.valid = true;

    return result;
}