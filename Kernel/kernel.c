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
	//char buffer[10];

	//ncPrint("[x64BareBones]");
	//ncNewline();

	//ncPrint("CPU Vendor:");
	//ncPrint(cpuVendor(buffer));
	//ncNewline();

	//ncPrint("[Loading modules]");
	//ncNewline();
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	//ncPrint("[Done]");
	//ncNewline();
	//ncNewline();

	//ncPrint("[Initializing kernel's binary]");
	//ncNewline();

	clearBSS(&bss, &endOfKernel - &bss);

	//ncPrint("  text: 0x");
	//ncPrintHex((uint64_t)&text);
	//ncNewline();
	//ncPrint("  rodata: 0x");
	//ncPrintHex((uint64_t)&rodata);
	//ncNewline();
	//ncPrint("  data: 0x");
	//ncPrintHex((uint64_t)&data);
	//ncNewline();
	//ncPrint("  bss: 0x");
	//ncPrintHex((uint64_t)&bss);
	//ncNewline();

	//ncPrint("[Done]");
	//ncNewline();
	//ncNewline();
	return getStackBase();
}

	uint32_t rainbowColor(int i) {
		uint8_t r = 0, g = 0, b = 0;
		int phase = i % 1536;

		if (phase < 256) {
			r = 255;
			g = phase;
		} else if (phase < 512) {
			r = 511 - (phase - 256);
			g = 255;
		} else if (phase < 768) {
			g = 255;
			b = phase - 512;
		} else if (phase < 1024) {
			g = 1023 - (phase - 768);
			b = 255;
		} else if (phase < 1280) {
			r = phase - 1024;
			b = 255;
		} else {
			r = 255;
			b = 1535 - phase;
		}

		return (r << 16) | (g << 8) | b;
	}

#define WHITE 0x00FFFFFF

void printSlow(char * str, uint32_t color, uint64_t pause){
	for(int i = 0; str[i]; i++){
		putChar(str[i], color);
		wait_ticks(pause);
	}
}

int main()
{	
	/*
	fontSizeUp(2);
	printStr(" TPE ARQUI \n", WHITE);
	fontSizeDown(2);
	load_idt();
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



