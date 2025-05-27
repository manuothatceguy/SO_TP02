#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

typedef struct memInfo {
    uint64_t total;
    uint64_t used;
    uint64_t free;
} memInfo;

typedef void (*fnptr)(uint64_t argc, char **argv);

typedef struct ProcessCreationParams {
    char* name;
    uint64_t function;
    uint64_t argc;
    char** arg;
    uint8_t priority;
} ProcessCreationParams;

#endif 