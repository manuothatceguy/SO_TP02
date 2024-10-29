#include <clock.h>
#include <lib.h>
#include <naiveConsole.h>
#include <stdint.h>
#include <interrupts.h>

#include <textModule.h> // BORRAR

#define NMI_DISABLE_BIT 1

unsigned int rtc(unsigned char reg){
    _cli();
    outb(0x70, reg | 0x80);
    outb(0x71, 0x20);
    _sti();
    return inb(0x71);
}

enum RTC_REGS {SECONDS = 0x00, MINUTES = 0x02, HOURS = 0x04, DAY_OF_WEEK = 0x06, DAY_OF_MONTH = 0x07, MONTH = 0x08, YEAR = 0x09};

unsigned int BCDToDecimal(unsigned char time);

unsigned int seconds(){
    return BCDToDecimal(rtc(SECONDS));
}

unsigned int minutes(){
    return BCDToDecimal(rtc(MINUTES));
}

unsigned int hours(){
    return BCDToDecimal(rtc(HOURS));
}

unsigned int day(){
    return BCDToDecimal(rtc(DAY_OF_MONTH));
}

unsigned int month(){
    return BCDToDecimal(rtc(MONTH));
}

unsigned int year(){
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

unsigned int BCDToDecimal(unsigned char bcd) {
    return (bcd >> 4)*10 + bcd%16;
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

uint64_t getTimeParam(uint64_t param) { // hacer que parezca menos del gordo
    int sec = seconds();
    int min = minutes();
    int hour = hours() + GMT_ARG;
    int dayVar = day();
    int monthVar = month();
    int yearVar = year();


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

    // Devolver el parámetro solicitado
    switch (param) {
        case 0: return sec;
        case 1: return min;
        case 2: return hour;
        case 3: return dayVar;
        case 4: return monthVar;
        case 5: return yearVar;
        default: return 0;
    }
}

