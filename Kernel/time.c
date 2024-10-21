#include <time.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

int wait_seconds(int secs){
	int curr = seconds_elapsed();
	while(seconds_elapsed() < curr+secs);
	return secs;
}

int wait_ticks(int ticks){
	int curr = ticks_elapsed();
	while(ticks_elapsed() < curr + ticks);
	return ticks;
}