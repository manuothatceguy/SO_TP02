#ifndef buddy
#include <memoryManager.h>

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/MemoryManager.c

typedef struct MemoryManagerCDT {
	char *nextAddress;
	uint32_t used;
	uint32_t total;
	memStatus status;
} MemoryManagerCDT;

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, uint64_t memorySize ) {
	MemoryManagerADT memoryManager = (MemoryManagerADT) memoryForMemoryManager;
	memoryManager->nextAddress = managedMemory;

	memoryManager->used = 0;
	memoryManager->total = memorySize;

	memoryManager->status.totalMemory = memorySize;
	memoryManager->status.freeMemory = memorySize;
	memoryManager->status.usedMemory = 0;

	return memoryManager;
}

void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate) {
	if ( memoryManager->used + memoryToAllocate > memoryManager->total ){
		return NULL;
	}

	char *allocation = memoryManager->nextAddress;

	memoryManager->nextAddress += memoryToAllocate;
	memoryManager->status.freeMemory -= memoryToAllocate;
	memoryManager->status.usedMemory += memoryToAllocate;
	
	return (void *) allocation;
}

memStatus *getMemStatus(MemoryManagerADT const restrict memoryManager){
	memStatus *status = allocMemory(memoryManager, sizeof(memStatus));
	if ( status == NULL ){
		return NULL;
	}
	status->totalMemory = memoryManager->status.totalMemory;
	status->usedMemory = memoryManager->status.usedMemory;
	status->freeMemory = memoryManager->status.freeMemory;

	return status;
}

void *freeMemory(MemoryManagerADT const restrict memoryManager, void *const restrict memoryToFree) {
	// Este memory manager es dummy, no soporta liberación de memoria
	return NULL;
}

#endif