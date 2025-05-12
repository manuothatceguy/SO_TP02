#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <processLinkedList.h>
#include <pcb.h>

uint64_t schedule(uint64_t rsp);
void initScheduler(ProcessLinkedPtr list);
// ... 

#endif // SCHEDULER_H