#ifndef BUDDY
#include <memoryManager.h>
#include <defs.h>

// src: https://github.com/alejoaquili/c-unit-testing-example/blob/main/src/MemoryManager.c

typedef struct MemoryManager {
	char *nextAddress;
	uint32_t used;
	uint32_t total;
} MemoryManager;

static MemoryManager *memoryManager = NULL;

void createMemoryManager() {
	memoryManager = (MemoryManager *) MEMORY_MANAGER_ADDRESS;
	memoryManager->nextAddress = (char *) HEAP_START_ADDRESS;

	memoryManager->used = 0;
	memoryManager->total = HEAP_SIZE;

}

void *allocMemory(const size_t memoryToAllocate) {
	if( memoryManager == NULL  || memoryToAllocate == 0 || memoryManager->used + memoryToAllocate > memoryManager->total ){
		return NULL; // tenes que tener un memory manager creado, sino no anda.
	}

	char *allocation = memoryManager->nextAddress;

	memoryManager->nextAddress += memoryToAllocate;

	return (void *) allocation;
}

void *freeMemory(void *const restrict memoryToFree) {
	// Este memory manager es dummy, no soporta liberación de memoria
	return NULL;
}


void getMemoryInfo(memInfo *info) {
    if (info == NULL) {
        return;
    }
    info->total = memoryManager->total;
    info->used = memoryManager->used;
    info->free = info->total - info->used;
}

#endif