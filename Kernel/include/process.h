#ifndef PROCESS_H
#define PROCESS_H

#include <pcb.h>

typedef struct ProcessList *ProcessLinkedPtr;

ProcessLinkedPtr createProcessLinkedList();
void addProcess(ProcessLinkedPtr list, PCB *process);
void removeProcess(ProcessLinkedPtr list, PCB *process);
void freeProcessLinkedList(ProcessLinkedPtr list);
PCB* getProcess(ProcessLinkedPtr list, pid_t pid);
PCB* getNextProcess(ProcessLinkedPtr list);
PCB* getCurrentProcess(ProcessLinkedPtr list);

// asm
uint64_t processStackFrame(uint64_t base, uint64_t rip, uint64_t argc, char **arg);


#endif // PROCESS_H