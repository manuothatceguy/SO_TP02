#ifndef PROCESS_H
#define PROCESS_H

//#include <pcb.h>
#include <shared_structs.h>

#define STACK_SIZE 4096 

typedef struct ProcessList *ProcessLinkedPtr;

ProcessLinkedPtr createProcessLinkedList();
void addProcess(ProcessLinkedPtr list, PCB *process);
void removeProcess(ProcessLinkedPtr list, pid_t pid);
void freeProcessLinkedList(ProcessLinkedPtr list);
PCB* getProcess(ProcessLinkedPtr list, pid_t pid);
PCB* getNextProcess(ProcessLinkedPtr list);
PCB* getCurrentProcess(ProcessLinkedPtr list);
uint64_t getProcessCount(ProcessLinkedPtr list);

// asm
uint64_t processStackFrame(uint64_t base, uint64_t rip, uint64_t argc, char **arg);


#endif // PROCESS_H