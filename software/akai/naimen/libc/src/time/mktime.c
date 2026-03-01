#include <limits.h>
#include <time.h>

static const int mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static int is_leap(int year1900)
{
    int y = year1900 + 1900;
    return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

time_t mktime(struct tm *t)
{
    long          sec  = t->tm_sec;
    long          min  = t->tm_min;
    long          hour = t->tm_hour;
    long          day  = t->tm_mday;
    long          mon  = t->tm_mon;
    long          year = t->tm_year;
    long          ymon;
    unsigned long days, total, add, yday;

    /* Normalize seconds → minutes */
    min += sec / 60;
    sec %= 60;
    if (sec < 0) sec += 60, min--;

    /* Normalize minutes → hours */
    hour += min / 60;
    min %= 60;
    if (min < 0) min += 60, hour--;

    /* Normalize hours → days */
    day += hour / 24;
    hour %= 24;
    if (hour < 0) hour += 24, day--;

    /* Normalize months → years */
    ymon = mon / 12;
    mon -= ymon * 12;
    if (mon < 0) mon += 12, ymon--;
    year += ymon;

    /* Reject years before 1970 */
    if (year < 70) return 0; /* clamp to epoch */

    /* Reject years too large for 32‑bit seconds */
    if (year > 138) /* ~2108 */
        return ULONG_MAX;

    /* Compute days since 1970‑01‑01 */
    days = 0;

    {
        long y, m;

        /* Add whole years */
        for (y = 70; y < year; y++) days += is_leap(y) ? 366UL : 365UL;

        /* Add whole months */
        for (m = 0; m < mon; m++) {
            days += mdays[m];
            if (m == 1 && is_leap(year)) days++;
        }
    }

    /* Add days in month */
    if (day < 1) return 0; /* clamp */
    days += (unsigned long)(day - 1);

    /* Convert to seconds */
    total = days * 86400UL;
    add   = hour * 3600UL + min * 60UL + sec;

    /* Overflow check */
    if (total > ULONG_MAX - add)
        total = ULONG_MAX;
    else
        total += add;

    /* Fill tm fields */
    t->tm_year = year;
    t->tm_mon  = mon;
    t->tm_mday = day;
    t->tm_hour = hour;
    t->tm_min  = min;
    t->tm_sec  = sec;

    /* weekday: 1970‑01‑01 was Thursday (4) */
    t->tm_wday = (days + 4) % 7;

    /* yday */

    yday = 0;
    {
        long m;
        for (m = 0; m < mon; m++) {
            yday += mdays[m];
            if (m == 1 && is_leap(year)) yday++;
        }
    }
    yday += (day - 1);
    t->tm_yday = (int)yday;

    t->tm_isdst = -1; /* unknown */

    return total;
}
