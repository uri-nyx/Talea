#include <time.h>

void _Totm(const time_t *timep, struct tm *out)
{
    static const int mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int              leap;
    unsigned long    t, sec, min, hour, days, wday, year, month, mday, yday;

    t   = (unsigned long)*timep;
    sec = t % 60;
    t /= 60;
    min = t % 60;
    t /= 60;
    hour = t % 24;
    t /= 24;
    days = t; // days since 1970‑01‑01
    wday = (days + 4) % 7;

    year = 1970;
    while (1) {
        int days_in_year = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 366 : 365;
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }

    leap  = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    month = 0;
    yday  = 0;
    while (1) {
        int dim = mdays[month];
        if (month == 1 && leap) dim++; // February in leap year
        if (days < dim) break;
        days -= dim;
        yday += dim;
        month++;
    }

    mday = days + 1;

    out->tm_sec  = sec;
    out->tm_min  = min;
    out->tm_hour = hour;
    out->tm_mday = mday;
    out->tm_mon  = month;
    out->tm_year = year - 1900;
    out->tm_wday = wday;
    out->tm_yday = yday;
    out->tm_isdst = -1;
}
