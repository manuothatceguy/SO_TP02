#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/include/MemoryManager.h

#include <stdlib.h>

typedef struct MemoryManagerCDT *MemoryManagerADT;

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory);

void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate);

#endif