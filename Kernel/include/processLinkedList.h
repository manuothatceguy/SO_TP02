#ifndef PROCESS_LINKED_LIST_H
#define PROCESS_LINKED_LIST_H

#include <pcb.h>

typedef struct ProcessList *ProcessLinkedPtr;

ProcessLinkedPtr createProcessLinkedList();
void addProcess(ProcessLinkedPtr list, PCB *process);
void removeProcess(ProcessLinkedPtr list, PCB *process);
void freeProcessLinkedList(ProcessLinkedPtr list);
PCB* getNextProcess(ProcessLinkedPtr list);
#endif // PROCESS_LINKED_LIST_H