#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>

typedef struct Process { // PCB
	uint16_t pid;             
	uint16_t parentPid;
	Code program;                
	char **args;                 
	char *name;                  
	uint8_t priority;         
	int16_t fileDescriptors[10]; // Descriptores de archivo TAMAÑO A CHEQUEAR

    void *stackBase; // MemoryBlock
	void *stackPos;
    
} Process;

typedef int (*Code)(int argc, char **args); // Puntero a la función del programa

void initProcess(Process *process, uint16_t pid, uint16_t parentPid, Code program, char **args, char *name, uint8_t priority, int16_t fileDescriptors[]);
void freeProcess(Process *process);
void closeFileDescriptors(Process *process);
#endif