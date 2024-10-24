#include <clock.h>
#include <lib.h>
#include <naiveConsole.h>

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
}

unsigned int BCDToDecimal(unsigned char bcd) {
    return (bcd >> 4)*10 + bcd%16;
}

time getTime(int timeZone){
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
}
