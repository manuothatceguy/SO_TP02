#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

#define NAME_MAX_LENGTH 32  

typedef struct memInfo {
    uint64_t total;
    uint64_t used;
    uint64_t free;
} memInfo;

typedef uint64_t (*fnptr)(uint64_t argc, char **argv);

typedef struct ProcessCreationParams {
    char* name;
    fnptr function;
    uint64_t argc;
    uint8_t priority;
    char** arg;
} ProcessCreationParams;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    EXITED,
    KILLED,
    WAITING_SEM
} ProcessState;

typedef int pid_t;

typedef struct PCB {
    pid_t pid; 
    pid_t parentPid;
    uint8_t priority; 
    ProcessState state; 
    uint64_t rsp;
    uint64_t base;
    uint64_t rip;
    char name[NAME_MAX_LENGTH];
    uint64_t retValue;
} PCB;


#endif 