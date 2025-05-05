#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/include/MemoryManager.h

#include <stdlib.h>
#include <stdint.h>

typedef struct MemoryManagerCDT * MemoryManagerADT;

typedef struct memStatus
{
    uint64_t totalMemory;
    uint64_t freeMemory;
    uint64_t usedMemory;
} memStatus;


/**
 * @brief Creates a memory manager that will manage the memory passed as an argument.
 * @param memoryForMemoryManager Pointer to the memory that will be used for the memory manager.
 * @param managedMemory Pointer to the memory that will be managed by the memory manager.
 * @param memorySize Size of the memory that will be managed by the memory manager.
 */
MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, uint64_t memorySize);
void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate);
void *freeMemory(MemoryManagerADT const restrict memoryManager, void *const restrict memoryToFree);
memStatus *getMemStatus(MemoryManagerADT const restrict memoryManager);
#endif