#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <processLinkedList.h>
#include <pcb.h>

uint64_t schedule(uint64_t rsp);
void initScheduler(ProcessLinkedPtr list);
pid_t createProcess(char* name, void(*function)(void*),uint64_t argc, char **arg, uint8_t priority);
pid_t getCurrentPid();
// ... 

#endif // SCHEDULER_H