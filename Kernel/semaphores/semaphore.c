#include <semaphore.h>
#include <stddef.h>
#include <memoryManager.h>
#include <queue.h>
#include <scheduler.h>
#include <shared_structs.h>

typedef struct SemaphoreCDT {
    sem_t semaphores[NUM_SEMS];
} SemaphoreCDT;

SemaphoreADT manager = NULL;

static SemaphoreADT getSemManager();

#define validateid(id) \
    do{ \
        if (id < 0 || id >= NUM_SEMS) { \
            return -1; \
        } \
    } while(0) 
    

SemaphoreADT semManager(){
    manager = (SemaphoreADT)(void*)allocMemory(sizeof(SemaphoreCDT));

    for (int i = 0; i < NUM_SEMS; i++) {
        manager->semaphores[i].value = 0;
        manager->semaphores[i].lock = 0;
        manager->semaphores[i].blocked = createQueue();

        if (manager->semaphores[i].blocked == NULL) {
            freeMemory(manager);
            return NULL;
        }
    }
    return manager;
}


int semInit (int id, uint32_t value) {
    validateid(id);

    SemaphoreADT manager = getSemManager();
    if( ! manager->semaphores[id].used ) {
        manager->semaphores[id].value = value;
        manager->semaphores[id].lock = 0;
        manager->semaphores[id].used = 1;
        return 0;
    }
    return -1;
}

int semOpen (int id) {
    validateid(id);
    SemaphoreADT manager = getSemManager();
    if ( ! manager->semaphores[id].used == 0 ) {
        return 0;
    }
    return -1;
}

int semClose(int id) {
    validateid(id);
	SemaphoreADT manager = getSemManager();
	if (! manager->semaphores[id].used) {
		return -1;
    }
	freeQueue(manager->semaphores[id].blocked);
    manager->semaphores[id].used = 0;
    manager->semaphores[id].value = 0;
    manager->semaphores[id].lock = 0;
    return 0;
}

static SemaphoreADT getSemManager() {
    return manager;
}

int semWait (int id) {
    SemaphoreADT manager = getSemManager();
    return wait(&manager->semaphores[id]);
}

int semPost (int id) {
    SemaphoreADT manager = getSemManager();
    return post(&manager->semaphores[id]);
}

int wait (sem_t *sem){
    acquire(&sem->lock);
    if (sem->value > 0) {
        sem->value--;
        release(&sem->lock);
        return 0;
    }
    pid_t *pid = (pid_t *)allocMemory(sizeof(pid_t));
    if (pid == NULL) {
        release(&sem->lock);
        return -1; // Memory allocation failed
    }
    *pid = getCurrentPid();
    enqueue(sem->blocked, (void*)pid);
    release(&sem->lock);
    blockProcess(*pid);
    return 0;
}

int post (sem_t *sem){
    acquire(&sem->lock);
    pid_t *pidPtr = (pid_t *)dequeue(sem->blocked);
    if (pidPtr != NULL) {
        pid_t pid = *pidPtr;
        freeMemory(pidPtr);  // Free the memory allocated in wait()
        unblockProcess(pid);
        release(&sem->lock);
        return 0;   
    }
    sem->value++;
    release(&sem->lock);
    return 0;
}

