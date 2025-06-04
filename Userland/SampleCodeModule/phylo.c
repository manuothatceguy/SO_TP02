#include <phylo.h>
#include <stdint.h>
#include <stdlib.h>
#include <syscall.h>

#define MAX_PHILOSOPHERS 10
#define TIME 5 //el tiempo de pensar y comer es el mismo(??

typedef enum {THINKING, HUNGRY, EATING} state;

typedef struct philosopher {
    pid_t pid;
    uint8_t id;
    int state;
    char **args;
    char *semName;
    char **descriptors;
    int status;
} Philosopher;

typedef struct status {
    char *mutex;
    uint64_t philosopherCount;
    uint64_t maxPhilosophers;
    Philosopher **philosophers;
    int *forks;  // Array of semaphore IDs for forks
    int mutex_sem;  // Mutex semaphore ID
} PhyloProcess;

static PhyloProcess *phylo_status = NULL;

static void take_forks(int id) {
    syscall_sem_wait(phylo_status->mutex_sem);
    
    //Hambre
    phylo_status->philosophers[id]->state = HUNGRY;
    
    //espero para tomar tenedores
    syscall_sem_wait(phylo_status->forks[id]);  
    syscall_sem_wait(phylo_status->forks[(id + 1) % phylo_status->philosopherCount]);  
    
    //comiendo
    phylo_status->philosophers[id]->state = EATING;
    
    syscall_sem_post(phylo_status->mutex_sem);
}

static void put_forks(int id) {
    syscall_sem_wait(phylo_status->mutex_sem);
    
    //pensando
    phylo_status->philosophers[id]->state = THINKING;
    
    //dejo tenedores
    syscall_sem_post(phylo_status->forks[id]);  
    syscall_sem_post(phylo_status->forks[(id + 1) % phylo_status->philosopherCount]);  
    
    //libero mutex
    syscall_sem_post(phylo_status->mutex_sem);
}

static void philosopher(uint64_t argc, char **argv) {
    if (argc != 1) {
        syscall_exit();
        return;
    }
    
    int id = atoi(argv[0]);
    if (id < 0 || id >= phylo_status->philosopherCount) {
        syscall_exit();
        return;
    }
    
    while (1) {
        // Think
        phylo_status->philosophers[id]->state = THINKING;
        printf("Philosopher %d is thinking\n", id);
        syscall_wait(THINKING_TIME);
        
        // Get hungry and eat
        take_forks(id);
        printf("Philosopher %d is eating\n", id);
        syscall_wait(EATING_TIME);
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
    phylo_status->forks = (int *)syscall_allocMemory(sizeof(int) * count);
    
    if (phylo_status->philosophers == NULL || phylo_status->forks == NULL) {
        syscall_freeMemory(phylo_status);
        return;
    }
    
    // Initialize mutex semaphore
    phylo_status->mutex_sem = 0;  // Use semaphore ID 0 for mutex
    if (syscall_sem_open(phylo_status->mutex_sem, 1) < 0) {
        syscall_freeMemory(phylo_status->philosophers);
        syscall_freeMemory(phylo_status->forks);
        syscall_freeMemory(phylo_status);
        return;
    }
    
    // Initialize fork semaphores
    for (int i = 0; i < count; i++) {
        phylo_status->forks[i] = i + 1;  // Use semaphore IDs 1 to count for forks
        if (syscall_sem_open(phylo_status->forks[i], 1) < 0) {
            // Clean up previously created semaphores
            for (int j = 0; j < i; j++) {
                syscall_sem_close(phylo_status->forks[j]);
            }
            syscall_sem_close(phylo_status->mutex_sem);
            syscall_freeMemory(phylo_status->philosophers);
            syscall_freeMemory(phylo_status->forks);
            syscall_freeMemory(phylo_status);
            return;
        }
    }
    
    // Create philosopher processes
    for (int i = 0; i < count; i++) {
        phylo_status->philosophers[i] = (Philosopher *)syscall_allocMemory(sizeof(Philosopher));
        if (phylo_status->philosophers[i] == NULL) {
            // Clean up
            for (int j = 0; j < i; j++) {
                syscall_freeMemory(phylo_status->philosophers[j]);
            }
            for (int j = 0; j < count; j++) {
                syscall_sem_close(phylo_status->forks[j]);
            }
            syscall_sem_close(phylo_status->mutex_sem);
            syscall_freeMemory(phylo_status->philosophers);
            syscall_freeMemory(phylo_status->forks);
            syscall_freeMemory(phylo_status);
            return;
        }
        
        // Initialize philosopher
        phylo_status->philosophers[i]->id = i;
        phylo_status->philosophers[i]->state = THINKING;
        
        // Create process for philosopher
        char id_str[2];
        id_str[0] = '0' + i;
        id_str[1] = '\0';
        char *argv[] = {id_str, NULL};
        
        pid_t pid = syscall_create_process("philosopher", philosopher, 1, argv, 1, 0);
        if (pid < 0) {
            // Clean up
            for (int j = 0; j <= i; j++) {
                syscall_freeMemory(phylo_status->philosophers[j]);
            }
            for (int j = 0; j < count; j++) {
                syscall_sem_close(phylo_status->forks[j]);
            }
            syscall_sem_close(phylo_status->mutex_sem);
            syscall_freeMemory(phylo_status->philosophers);
            syscall_freeMemory(phylo_status->forks);
            syscall_freeMemory(phylo_status);
            return;
        }
        
        phylo_status->philosophers[i]->pid = pid;
    }
}

uint64_t phylo(uint64_t argc, char **argv) {
    if (argc != 2) {
        printf("Uso: philo <max_number>\n");
        return 1;
    }
    
    uint64_t count = atoi(argv[1]);
    if (count <= 0 || count > MAX_PHILOSOPHERS) {
        printf("El número de filósofos debe estar entre 1 y %d\n", MAX_PHILOSOPHERS);
        return 1;
    }
    
    init_philosophers(count);
    
    if (phylo_status == NULL) {
        printf("Error al inicializar los filósofos\n");
        return 1;
    }
    
    // Wait for all philosophers to finish (they won't in this case as they run forever)
    for (int i = 0; i < count; i++) {
        int32_t status;
        syscall_waitpid(phylo_status->philosophers[i]->pid, &status);
    }
    
    return 0;
}


