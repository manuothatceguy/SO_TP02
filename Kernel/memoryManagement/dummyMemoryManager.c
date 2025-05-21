#ifndef BUDDY
#include <memoryManager.h>
#include <defs.h>

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/MemoryManager.c

typedef struct MemoryManager {
	char *nextAddress;
	uint64_t used;
	uint64_t total;
	memStatus status;
} MemoryManager;

static MemoryManager *memoryManager = NULL;

void createMemoryManager() {
	memoryManager = (MemoryManager *) MEMORY_MANAGER_ADDRESS;
	memoryManager->nextAddress = (char *) HEAP_START_ADDRESS;

	memoryManager->used = 0;
	memoryManager->total = HEAP_SIZE;

	memoryManager->status.totalMemory = HEAP_SIZE;
	memoryManager->status.freeMemory = HEAP_SIZE;
	memoryManager->status.usedMemory = 0;

}

void *allocMemory(const size_t memoryToAllocate) {
	if( memoryManager == NULL  || memoryToAllocate == 0 || memoryManager->used + memoryToAllocate > memoryManager->total ){
		return NULL; // tenes que tener un memory manager creado, sino no anda.
	}

	char *allocation = memoryManager->nextAddress;

	memoryManager->nextAddress += memoryToAllocate;
	memoryManager->status.freeMemory -= memoryToAllocate;
	memoryManager->status.usedMemory += memoryToAllocate;
	
	return (void *) allocation;
}

memStatus *getMemStatus(){
	memStatus *status = allocMemory(sizeof(memStatus));
	if ( status == NULL ){
		return NULL;
	}
	status->totalMemory = memoryManager->status.totalMemory;
	status->usedMemory = memoryManager->status.usedMemory;
	status->freeMemory = memoryManager->status.freeMemory;

	return status;
}

void *freeMemory(void *const restrict memoryToFree) {
	// Este memory manager es dummy, no soporta liberación de memoria
	return NULL;
}

#endif