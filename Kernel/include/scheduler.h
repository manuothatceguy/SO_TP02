#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <process.h>
#include <pcb.h>
#include <defs.h>


void initScheduler();

pid_t getCurrentPid();

uint64_t schedule(uint64_t rsp);

pid_t createProcess(char* name, fnptr function, char **argv, uint8_t priority);

uint64_t blockProcess(pid_t pid);
uint64_t unblockProcess(pid_t pid);

void yield();
uint64_t kill(pid_t pid);
int8_t changePrio(pid_t pid, int8_t newPrio);

PCB* getProcessInfo(uint64_t *cantProcesses);
int16_t copyProcess(PCB *dest, PCB *src);
// ... 

#endif // SCHEDULER_H