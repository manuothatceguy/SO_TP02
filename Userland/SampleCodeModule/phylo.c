#include <phylo.h>
#include <stdint.h>
#include <stdlib.h>
#include <syscall.h>

#define MUTEX_ID 50
#define MAX_PHILOSOPHERS 10
#define TIME 2 //el tiempo de pensar y comer es el mismo(??

typedef enum {THINKING, HUNGRY, EATING} state;

typedef struct philosopher {
    pid_t pid;
    uint8_t id;
    int state;
    //char **args;
    int semName;
    //char **descriptors;
    //int status;
} Philosopher;

typedef struct status {
    //char *mutex;
    uint64_t philosopherCount;
    uint64_t maxPhilosophers;
    Philosopher **philosophers;
    //int *forks;  // Array of semaphore IDs for forks
    int mutex_sem;  // Mutex semaphore ID
} PhyloProcess;

static PhyloProcess *phylo_status = NULL;

static int getRightBlock(int id) {
  syscall_sem_wait(phylo_status->mutex_sem);
  int right = (id + 1) % phylo_status->philosopherCount;
  syscall_sem_post(phylo_status->mutex_sem);
  return right;
}

static void print_state(void) {
    syscall_sem_wait(phylo_status->mutex_sem);
    uint64_t currentCount = phylo_status->philosopherCount;
    for (int i = 0; i < currentCount && i < MAX_PHILOSOPHERS; i++) {
        printf("%s", phylo_status->philosophers[i]->state == EATING ? "E" : ".");
    }
    printf("\n");
    syscall_sem_post(phylo_status->mutex_sem);
}


static void take_forks(int id) {
  if (id % 2 == 0) {
    syscall_sem_wait(phylo_status->philosophers[id]->semName);
    syscall_sem_wait(phylo_status->philosophers[getRightBlock(id)]->semName);
  } else {
    syscall_sem_wait(phylo_status->philosophers[getRightBlock(id)]->semName);
    syscall_sem_wait(phylo_status->philosophers[id]->semName);
  }
}

static void put_forks(int id) {
  if (id % 2 == 0) {
    syscall_sem_post(phylo_status->philosophers[getRightBlock(id)]->semName);
    syscall_sem_post(phylo_status->philosophers[id]->semName);
  } else {
    syscall_sem_post(phylo_status->philosophers[id]->semName);
    syscall_sem_post(phylo_status->philosophers[getRightBlock(id)]->semName);
  }
}

static uint64_t philosopher(uint64_t argc, char *argv[]) {
    int id = atoi(argv[0]);
    if (id < 0 || id >= phylo_status->philosopherCount) {
        syscall_exit();
        return 1;
    }
    
  while (1) {
    syscall_sem_wait(phylo_status->mutex_sem);
    phylo_status->philosophers[id]->state = THINKING;
    syscall_sem_post(phylo_status->mutex_sem);

    syscall_wait(TIME);

    // Hambriento
    syscall_sem_wait(phylo_status->mutex_sem);
    phylo_status->philosophers[id]->state = HUNGRY;
    syscall_sem_post(phylo_status->mutex_sem);

    take_forks(id);

    // Comiendo
    syscall_sem_wait(phylo_status->mutex_sem);
    phylo_status->philosophers[id]->state = EATING;
    syscall_sem_post(phylo_status->mutex_sem);

    print_state();

    syscall_wait(TIME);

    // Pensando otra vez
    syscall_sem_wait(phylo_status->mutex_sem);
    phylo_status->philosophers[id]->state = THINKING;
    syscall_sem_post(phylo_status->mutex_sem);

    put_forks(id);
  }
}

static void init_philosophers(uint64_t count) {
    if (count > MAX_PHILOSOPHERS) {
        count = MAX_PHILOSOPHERS;
    }
    
    // Allocate memory for the status structure
    phylo_status = (PhyloProcess *)syscall_allocMemory(sizeof(PhyloProcess));
    if (phylo_status == NULL) {
        return;
    }
    
    // Initialize the status structure
    phylo_status->philosopherCount = count;
    phylo_status->maxPhilosophers = MAX_PHILOSOPHERS;
    phylo_status->philosophers = (Philosopher **)syscall_allocMemory(sizeof(Philosopher *) * count);

    if (phylo_status->philosophers == NULL ) { 
        syscall_freeMemory(phylo_status);
        return;
    }
    
    // Initialize mutex semaphore
    phylo_status->mutex_sem = MUTEX_ID;
    if (syscall_sem_open(phylo_status->mutex_sem, 1) < 0) {
        syscall_freeMemory(phylo_status->philosophers);
        syscall_freeMemory(phylo_status);
        return;
    }
    // Create philosopher processes
    for (int i = 0; i < count; i++) {
        phylo_status->philosophers[i] = (Philosopher *)syscall_allocMemory(sizeof(Philosopher));
        if (phylo_status->philosophers[i] == NULL) {
            // Clean up
            for (int j = 0; j < i; j++) {
                syscall_freeMemory(phylo_status->philosophers[j]);
            }
            syscall_sem_close(phylo_status->mutex_sem);
            syscall_freeMemory(phylo_status->philosophers);
            syscall_freeMemory(phylo_status);
            return;
        }
        
        // Initialize philosopher
        phylo_status->philosophers[i]->id = i;
        phylo_status->philosophers[i]->state = THINKING;
        
        phylo_status->philosophers[i]->semName= MUTEX_ID + i + 1; 
        if (syscall_sem_open(phylo_status->philosophers[i]->semName, 1) < 0) {
            syscall_freeMemory(phylo_status->philosophers[i]);
            return;
        }
        
        // Create process for philosopher
        char *id_str = (char *)syscall_allocMemory(2);
        if (id_str == NULL) {
            return;
        }
        id_str[0] = '0' + i;
        id_str[1] = '\0';
        char *argv[] = {id_str, NULL};
        
        pid_t pid = syscall_create_process("philosopher", philosopher, 1, argv, 1, 0, 0, 1);
        if (pid < 0) {
            // Clean up
            for (int j = 0; j <= i; j++) {
                syscall_freeMemory(phylo_status->philosophers[j]);
            }
            syscall_sem_close(phylo_status->mutex_sem);
            syscall_freeMemory(phylo_status->philosophers);
            syscall_freeMemory(phylo_status);
            return;
        }
        
        phylo_status->philosophers[i]->pid = pid;
    }
}

void phylo(uint64_t argc, char **argv) {
    uint64_t count = atoi(argv[0]);
    if (count <= 0 || count > MAX_PHILOSOPHERS) {
        printf("El numero de filosofos debe estar entre 1 y %d\n", MAX_PHILOSOPHERS);
        return;
    }
    
    init_philosophers(count);
    
    if (phylo_status == NULL) {
        printf("Error al inicializar los filosofos\n");
        return;
    }
    
    return;
}