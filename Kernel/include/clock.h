#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

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

int64_t getTimeParam(int64_t tz,uint64_t param); // -1 si falla

#endif // CLOCK_H