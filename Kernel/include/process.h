#ifndef PROCESS_H
#define PROCESS_H

#include <pcb.h>
#include <defs.h>

#define MAX_PRIORITY 5 // agregar a limitaciones
#define MIN_PRIORITY 0
#define IDLE_PRIORITY -1

typedef struct ProcessList *ProcessListADT;

ProcessListADT createProcessLinkedList();
pid_t createReadyProcess(ProcessListADT list, char* name, fnptr function, char **argv, uint8_t priority);
pid_t readyToBlocked(ProcessListADT list, pid_t pid);
pid_t blockedToReady(ProcessListADT list, pid_t pid);
PCB* getRunningProcess(ProcessListADT list); // no avanza la lista
PCB* getNextProcess(ProcessListADT list); // avanza la lista y retorna el siguiente
int8_t changePriority(ProcessListADT list, pid_t pid, uint8_t newPrio);
/* repensar
void removeProcess(ProcessListADT list, pid_t pid);
void freeProcessLinkedList(ProcessListADT list);
PCB* getProcess(ProcessListADT list, pid_t pid);
PCB* getNextProcess(ProcessListADT list);
PCB* getCurrentProcess(ProcessListADT list);
*/

// asm
uint64_t processStackFrame(uint64_t base, uint64_t rip, uint64_t argc, char **arg);


#endif // PROCESS_H