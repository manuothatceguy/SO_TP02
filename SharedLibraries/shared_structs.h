#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

#define NAME_MAX_LENGTH 32  
#define NUM_SEMS 300

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
    int8_t priority;
    char** arg;
    char foreground;
    int stdin;
    int stdout;  
} ProcessCreationParams;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    EXITED,
    KILLED,
    WAITING_SEM
} ProcessState;

typedef int pid_t;

typedef struct {
    pid_t pid;
    pid_t parentPid;
    pid_t waitingForPid;
    int8_t priority;
    ProcessState state;
    uint64_t rsp;
    uint64_t base;
    uint64_t rip;
    uint64_t retValue;
    char foreground;
    struct {
        int stdin;
        int stdout;
    } fds;
    char name[NAME_MAX_LENGTH];
} PCB;


#endif 