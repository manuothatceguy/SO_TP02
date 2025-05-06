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
} time_t;

/**
 * RTC time
 * @param timeZone to adjust to the correct time zone. Must put the value as it is. May cause errors if
 * timeZone is invalid.
 */
time_t getTime(int64_t timeZone);

uint64_t getTimeParam(uint64_t param); // -1 si falla

#endif // CLOCK_H