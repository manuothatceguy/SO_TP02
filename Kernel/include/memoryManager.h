#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/include/MemoryManager.h

#include <stdlib.h>
#include <stdint.h>
#include "../../SharedLibraries/shared_structs.h"


void createMemoryManager();
void *allocMemory(const size_t memoryToAllocate);
void *freeMemory(void *const restrict memoryToFree);
void getMemoryInfo(memInfo *info);
#endif