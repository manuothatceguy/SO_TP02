#ifndef CLOCK_H
#define CLOCK_H

typedef struct time{
    int sec;
    int min;
    int hour;
    int day;
    int month;
    int year;
} time;

/**
 * RTC time
 * @param timeZone to adjust to the correct time zone. Must put the value as it is. May cause errors if
 * timeZone is invalid.
 */
time getTime(int timeZone);

#endif // CLOCK_H