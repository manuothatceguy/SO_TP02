#ifndef PROCESS_H
#define PROCESS_H

//#include <pcb.h>
#include <shared_structs.h>

#define STACK_SIZE 4096 

typedef struct ProcessManagerCDT *ProcessManagerADT;

ProcessManagerADT createProcessManager();

void addProcess(ProcessManagerADT list, PCB *process);

void removeProcess(ProcessManagerADT list, pid_t pid);

void freeProcessLinkedList(ProcessManagerADT list);

// busca en ambas colas
PCB* getProcess(ProcessManagerADT list, pid_t pid);

PCB* getNextProcess(ProcessManagerADT list);

PCB* getCurrentProcess(ProcessManagerADT list);

int blockProcessQueue(ProcessManagerADT list, pid_t pid);

int unblockProcessQueue(ProcessManagerADT list, pid_t pid);

uint64_t countReadyProcesses(ProcessManagerADT list);

uint64_t countBlockedProcesses(ProcessManagerADT list);

uint64_t countProcesses(ProcessManagerADT list);

PCB* killProcess(ProcessManagerADT list, pid_t pid, uint64_t retValue, ProcessState state);

// asm
uint64_t processStackFrame(uint64_t base, uint64_t rip, uint64_t argc, char **arg);
void wrapper(uint64_t function, char **argv);

#endif // PROCESS_H