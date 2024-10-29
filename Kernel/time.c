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
	// _sti(); Unicamente si se quiere que el timer siga corriendo mientras me encuentro en la interrupcion de teclado (int_21)
	uint64_t curr = ticks;
	while(ticks < curr + ticksToWait);
}

void setup_timer(uint16_t freq) {
    // Configurar PIT para el temporizador
    outb(0x43, 0xB6); // Modo de interrupción, frecuencia
    outb(0x40, 1193180 / freq); 
    outb(0x40, (1193180 / freq) >> 8); 
	frequency = freq;
}
