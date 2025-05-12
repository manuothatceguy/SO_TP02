#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/include/MemoryManager.h

#include <stdlib.h>
#include <stdint.h>

typedef struct memStatus
{
    uint64_t totalMemory;
    uint64_t freeMemory;
    uint64_t usedMemory;
} memStatus;


void createMemoryManager();
void *allocMemory(const size_t memoryToAllocate);
void *freeMemory(void *const restrict memoryToFree);
memStatus *getMemStatus();
#endif