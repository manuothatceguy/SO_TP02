#include <clock.h>
#include <lib.h>
#include <naiveConsole.h>
#include <stdint.h>
#include <interrupts.h>

#include <textModule.h> // BORRAR

#define Y2K 2000

uint8_t rtc(unsigned char reg) {
    _cli();
    outb(0x70, 128 | reg); // 128 = 10000000b
    _sti();
    return inb(0x71);
}

enum RTC_REGS {SECONDS = 0x00, MINUTES = 0x02, HOURS = 0x04, DAY_OF_MONTH = 0x07, MONTH = 0x08, YEAR = 0x09};

uint8_t BCDToDecimal(uint8_t time);

uint8_t seconds(){
    return BCDToDecimal(rtc(SECONDS));
}

uint8_t minutes(){
    return BCDToDecimal(rtc(MINUTES));
}

uint8_t hours(){
    return BCDToDecimal(rtc(HOURS));
}

uint8_t day(){
    return BCDToDecimal(rtc(DAY_OF_MONTH));
}

uint8_t month(){
    return BCDToDecimal(rtc(MONTH));
}

uint8_t year(){
    return BCDToDecimal(rtc(YEAR));
}

int isLeapYear(int year){ // From K&R - The C Programming Language
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}
/*
void checkMonth(time * time){
    if(time->month > 12){
        time->month = 1;
        time->year++;
    } else if(time->month <= 0){
        time->year--;
        time->month = 12;
    }
}

void checkDay(time * time){
    char daytab[2][13] = { // From K&R - The C Programming Language
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    if(time->day > daytab[isLeapYear(time->year)][time->month]){
        time->day = 1;
        time->month++;
    } else if(time->day <= 0){
        time->month--;
        checkMonth(time);
        time->day = daytab[isLeapYear(time->year)][time->month];
    }
}*/

uint8_t BCDToDecimal(uint8_t time) {
	return ((time & 0xF0) >> 4) * 10 + (time & 0x0F);
}
/*
time getTime(int64_t timeZone){
    time toReturn = {
        seconds(),
        minutes(),
        hours() + timeZone,
        day(),
        month(),
        year()
    };

    if(toReturn.hour > 24){
        toReturn.hour -= 24;
        toReturn.day += 1;
        checkDay(&toReturn);
        checkMonth(&toReturn);
    }
    else if(toReturn.hour < 0){
        toReturn.hour += 24;
        toReturn.day--;
        checkDay(&toReturn);
        checkMonth(&toReturn);
    }
    return toReturn;
}*/

void checkDay(int *day, int *month, int *year) {
    char daytab[2][13] = { // from K&R - The C Programming Language
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    
    int leap = isLeapYear(*year);
    
    if (*day > daytab[leap][*month]) {
        *day = 1;
        (*month)++;
        if (*month > 12) {
            *month = 1;
            (*year)++;
        }
    } else if (*day <= 0) {
        (*month)--;
        if (*month <= 0) {
            *month = 12;
            (*year)--;
        }
        *day = daytab[isLeapYear(*year)][*month];
    }
}

int getDaysInMonth(int month, int year) {
    char daytab[2][13] = {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    return daytab[isLeapYear(year)][month];
}


#define GMT_ARG -3

char* intToString(int num) {
    static char buffer[12]; // Enough to hold -2147483648 and null terminator
    int i = 10;
    buffer[11] = '\0';
    int isNegative = num < 0;

    if (isNegative) {
        num = -num;
    }

    do {
        buffer[i--] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    if (isNegative) {
        buffer[i--] = '-';
    }

    return &buffer[i + 1];
}



uint64_t getTimeParam(uint64_t param) { // hacer que parezca menos del gordo
    uint8_t sec = seconds();
    uint8_t min = minutes();
    uint8_t hour = hours() + GMT_ARG;
    uint8_t dayVar = day();
    uint8_t monthVar = month();
    uint16_t yearVar = year() + Y2K;


    // Ajustar la hora con cambio de día si es necesario
    if (hour >= 24) {
        hour -= 24;
        dayVar++;
    } else if (hour < 0) {
        hour += 24;
        dayVar--;
    }

    // Ajustar el día con cambio de mes si es necesario
    if (dayVar > getDaysInMonth(monthVar, yearVar)) {
        dayVar = 1;
        monthVar++;
        if (monthVar > 12) {
            monthVar = 1;
            yearVar++;
        }
    } else if (dayVar <= 0) {
        monthVar--;
        if (monthVar <= 0) {
            monthVar = 12;
            yearVar--;
        }
        dayVar = getDaysInMonth(monthVar, yearVar);
    }

  switch (param) {
        case 0: {
            return sec;
        } 
        case 1: {
            return min;
        }
        case 2: {
            return hour;
        }
        case 3: {
            return dayVar;
        }
        case 4: {
            return monthVar;
        }
        case 5: {
            return yearVar;
        }
        default: return 0;
    }
    return 0;
}

