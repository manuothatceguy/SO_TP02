#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <process.h>
//#include <pcb.h>
#include <defs.h>
#include <shared_structs.h>

uint64_t schedule(uint64_t rsp);
void initScheduler(fnptr idle);
pid_t createProcess(char* name, fnptr function, uint64_t argc, char **arg, uint8_t priority);
pid_t getCurrentPid();
uint64_t blockProcess (pid_t pid);
void yield();
uint64_t unblockProcess(pid_t pid);
uint64_t kill(pid_t pid);
pid_t waitpid(pid_t pid, int32_t* status);
int8_t changePrio(pid_t pid, int8_t newPrio);

PCB* getProcessInfo(uint64_t *cantProcesses);
int16_t copyProcess(PCB *dest, PCB *src);
// ... 

#endif // SCHEDULER_H