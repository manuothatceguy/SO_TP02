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
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <pipes.h>
#include <semaphore.h>

#define WHITE 0x00FFFFFF
#define RED 0x000000FF

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
	clearBSS(&bss, (uint64_t)(&endOfKernel) - (uint64_t)(&bss));
	return getStackBase();
}

void idle(uint64_t argc, char **argv) {
	while(1){
		_hlt();
	}
}

// Test process to verify pipe communication
void pipeTest(uint64_t argc, char **argv) {
    int pipe_id = createPipe();
    if (pipe_id < 0) {
        printStr("Error creating pipe\n", RED);
        return;
    }

    char test_msg[] = "Testing pipe communication...\n";
    char read_buffer[100] = {0};

    // Write to pipe
    int written = writePipe(pipe_id, test_msg, strlen(test_msg));
    if (written < 0) {
        printStr("Error writing to pipe\n", RED);
        return;
    }

    // Read from pipe
    int read = readPipe(pipe_id, read_buffer, written);
    if (read < 0) {
        printStr("Error reading from pipe\n", RED);
        return;
    }

    // Display what was read
    printStr("Read from pipe: ", WHITE);
    printStr(read_buffer, WHITE);

    closePipe(pipe_id);
}

int feDeVida(uint64_t argc, char **argv) {
	while(1) {
		printStr("se me jijean\n", WHITE);
		wait_seconds(4);
	}
	return 0;
}

int feDeVida2(uint64_t argc, char *argv[]) {
	while(1) {
		if (argc > 0 && argv != NULL && argv[0] != NULL) {
			printStr(argv[0], WHITE);
		} else {
			printStr("los jolines\n", WHITE);
		}
		wait_ticks(40);
	}
	return 0;
}

int main(){	
	_cli();
	fontSizeUp(2);
	printStr(" TP 2 SO \n", WHITE);
	fontSizeDown(2);

	clear_buffer();

	// MEMORY MANAGER
	createMemoryManager();
	initScheduler(idle);
	semManager();
	//createProcess("idle", &idle, 0, NULL, -1);
	//createProcess("feDeVida",(fnptr) feDeVida, 0, NULL, 1);
	//char *feDeVida2Args[] = {"Hola mundo!\n", NULL};
	//createProcess("feDeVida2", (fnptr) feDeVida2, 1, feDeVida2Args, 1);
	// Test pipe communication
	//createProcess("pipeTest", (fnptr)pipeTest, 0, NULL, 1, 1);
	// Crear pipe para stdin de la shell
	int shell_stdin = createPipe();
	char shell_stdin_str[8];
	itoa(shell_stdin, shell_stdin_str, 10);
	char *shell_args[] = { shell_stdin_str, NULL };
	createProcess("shell", (fnptr) sampleCodeModuleAddress, 1, shell_args, 0, 1);
	load_idt();
	_sti();
	while (1) {
		_hlt();
	}

	printStr("DEATH ZONE\n", RED);
	// This point should never be reached
	return 0;
}



