#include <semaphore.h>
#include <stddef.h>
#include <memoryManager.h>

#define NUM_SEMS 30

typedef struct sem {
	uint32_t value;
	uint8_t lock;
	// cola se procesos bloqueados esperadno
} sem_t;

typedef struct SemaphoreCDT {
	sem_t semaphores[NUM_SEMS];
} SemaphoreCDT;

SemaphoreADT manager = NULL;

SemaphoreADT semManager(){
    manager = (SemaphoreADT)(void*)allocMemory(sizeof(SemaphoreCDT));
    if (manager == NULL) {
        return NULL;
    }
    for (int i = 0; i < NUM_SEMS; i++) {
        manager->semaphores[i].value = 0;
        manager->semaphores[i].lock = 0;
    }
    return manager;
}