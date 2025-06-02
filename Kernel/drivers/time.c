#include <time.h>
#include <lib.h>
#include <textModule.h>
#include <interrupts.h>
#include <scheduler.h>
#include <clock.h>

static uint64_t ticks = 0;
static uint16_t frequency = 18;

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / frequency;
}

void wait_ticks(uint64_t ticksToWait) {
	//_sti();
	uint64_t curr = ticks;
	while(ticks < curr + ticksToWait){
		yield(); // antes era hlt pero tenemos procesos.....
	}
}

void wait_seconds(uint64_t secondsToWait) {
	time_t curr = getTime(0); // 0 para GMT
	while(diffTimeMillis(curr, getTime(0))/1000 < secondsToWait){
		//ticks--;
		yield();
		//printStr("Waiting for seconds...\n", 0x00FFFFFF);
	}
}

void setup_timer(uint16_t freq) {
    uint16_t divisor = 1193180 / freq;

    outb(0x43, 0x36);            
    outb(0x40, divisor & 0xFF);   
    outb(0x40, (divisor >> 8) & 0xFF); 

    frequency = freq;
}

