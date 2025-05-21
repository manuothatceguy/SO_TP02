#ifndef PCB_H
#define PCB_H

#include <stdint.h>

#define STACK_SIZE 4096 
#define NAME_MAX_LENGTH 32  

typedef int pid_t;

typedef struct PCB {
    pid_t pid; 
    pid_t parentPid;
    uint8_t priority; 
    uint64_t rsp;
    uint64_t base;
    char name[NAME_MAX_LENGTH];
} PCB;

#endif // PCB_H