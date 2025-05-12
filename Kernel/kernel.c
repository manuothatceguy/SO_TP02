#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <idtLoader.h>
#include <clock.h>
#include <time.h>
#include <videoDriver.h>
#include <textModule.h>
#include <keyboardDriver.h>
#include <memoryManager.h>
#include <defs.h>
#include <processLinkedList.h>

#define WHITE 0x00FFFFFF

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
		+ PageSize * 8				
		- sizeof(uint64_t)			
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

int main(){	
	
	load_idt();
	setup_timer(18);
	fontSizeUp(2);
	printStr(" TP 2 SO \n", WHITE);
	fontSizeDown(2);
	wait_ticks(40);

	
	clear_buffer();

	// MEMORY MANAGER
	createMemoryManager();
	//ProcessLinkedPtr processList = createProcessLinkedList();
	//addProcess(processList, NULL); // en vez de null deberia estar el proceso idle!!
	//initScheduler(processList);


	// reemplazar por tickeo forzado para usar el scheduler
	((EntryPoint)sampleCodeModuleAddress)(); // Llamada al userland
	clearScreen(0);
	
	return 0;
}



