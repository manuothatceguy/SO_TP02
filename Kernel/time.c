#include <time.h>
#include <lib.h>
#include <textModule.h>
#include <interrupts.h>

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
	_sti();   // Unicamente si se quiere que el timer siga corriendo mientras me encuentro en la interrupcion de teclado (int_21)
	uint64_t curr = ticks;
	while(ticks < curr + ticksToWait);
}

void setup_timer(uint16_t freq) {
    uint16_t divisor = 1193180 / freq;

    outb(0x43, 0x36);            
    outb(0x40, divisor & 0xFF);   
    outb(0x40, (divisor >> 8) & 0xFF); 

    frequency = freq;
}

