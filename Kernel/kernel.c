#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
//#include <naiveConsole.h>
#include <idtLoader.h>
#include <clock.h>
#include <time.h>
#include <videoDriver.h>
#include <textModule.h>
#include <keyboardDriver.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary()
{
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}


void printSlow(char * str, uint32_t color, uint64_t pause){
	for(int i = 0; str[i]; i++){
		putChar(str[i], color);
		wait_ticks(pause);
	}
}

int main()
{	
	load_idt();
	setup_timer(18);
	/*
	fontSizeUp(2);
	printStr(" TPE ARQUI \n", WHITE);
	fontSizeDown(2);
	
	setup_timer(18);
	printStr("\nConfigurando\n\n", WHITE);
	printSlow("    1- La IDT ya fue cargada\n", WHITE, 30);
	wait_ticks(40);
	printSlow("    2- Cargando la shell",WHITE, 30);
	printSlow("...",WHITE,2800);
	*/
	((EntryPoint)sampleCodeModuleAddress)(); // LLAMADA AL USERLAND
	clearScreen(0);

	return 0;
}



