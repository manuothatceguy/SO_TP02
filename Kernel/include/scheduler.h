#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <process.h>
#include <pcb.h>
#include <defs.h>

uint64_t schedule(uint64_t rsp);
void initScheduler(ProcessLinkedPtr list);
pid_t createProcess(char* name, fnptr function,uint64_t argc, char **arg, uint8_t priority);
pid_t getCurrentPid();
uint64_t blockProcess (pid_t pid);
void yield();
uint64_t unblockProcess(pid_t pid);
uint64_t kill(pid_t pid);
int8_t changePrio(pid_t pid, int8_t newPrio);
// ... 

#endif // SCHEDULER_H