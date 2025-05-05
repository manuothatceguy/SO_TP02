#ifndef PCB_H
#define PCB_H

#include <stdint.h>

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    EXITED
} ProcessState;

typedef struct PCB {
    int pid; 
    int priority; 
    ProcessState state; 
    uint64_t rip; 
    uint64_t rsp; 
    uint64_t registers[8]; // GPRs
} PCB;

#endif // PCB_H