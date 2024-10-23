#include <time.h>
#include <stdint.h>
#include <naiveConsole.h>
#include <keyboardDriver.h>
#include <textModule.h>

void int_20();
void int_21();

void irqDispatcher(uint64_t irq) {
	switch (irq) {
		case 0:
			int_20();
			break;
		case 1:
			int_21();
	}
	return;
}

void int_20(){
	timer_handler();
}

void int_21(){
	int c = bufferWrite();
	if(c != 0)
		putChar(getChar(),2,0x00FF0000); // BORRAR
}
