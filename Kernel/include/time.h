#ifndef _TIME_H_
#define _TIME_H_

void timer_handler();
int ticks_elapsed();
int seconds_elapsed();
int wait_seconds(int secs);
int wait_ticks(int ticks);

#endif
