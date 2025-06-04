#include <syscall.h>
#include <philo.h>
#include <stdlib.h>

#define MUTEX_ID 100
#define MAX_PHILO 10
#define MIN_PHILO 5
#define SECS 2

#define ADD 'a'
#define REMOVE 'r'
#define QUIT 'q'

#define LEFT(i) (((i) + count - 1) % count)
#define RIGHT(i) (((i) + 1) % count)

typedef enum { THINKING = 0, HUNGRY, EATING } philo_state;

philo_state states[MAX_PHILO];
uint16_t pids[MAX_PHILO];
uint8_t count = 0;

static void philosopher(int id);
static void take_forks(int id);
static void put_forks(int id);
static void test(int id);


int startPhilo () {
    if ( syscall_sem_open(MUTEX_ID, 1) < 0 ) {
        return -1;
    }

    for ( int i = 0; i < MAX_PHILO; i++){
        pids[i] = i;
    }

    for ( int i = 0; i < MIN_PHILO; i++) {
        addPhilosopher();
    }

    char c;
    while ( (c = getChar()) != QUIT ) {
        switch (c) {
            case ADD: 
                if ( count < MAX_PHILO ) {
                    if ( addPhilosopher() == -1 ){
                        printf("Error al agregar filosofo\n");
                    }
                }
                else {
                    printf("No se pueden agregar más filosofos\n");
                }
                break;
            case REMOVE:
                if ( count > MIN_PHILO ) {
                    if ( removePhilosopher() == -1 ) {
                        printf("Error al eliminar filosofo\n");
                    }
                }
                else {
                    printf("No se pueden eliminar mas filosofos\n");
                }
                break;    
        }
    }

    for ( int i = 0; i < count; i++ ) {
        removePhilosopher(pids[i]);
    }

    syscall_sem_close(MUTEX_ID);
    return 0;
}

int addPhilosopher() {
    return 0;
}

int removePhilosopher() {
    return 0;
}

static void philosopher (int id) {
    while(1){
        syscall_wait(SECS);
        take_forks(id);
        syscall_wait(SECS);
        put_forks(id);
    }
}

static void take_forks(int id) {
    syscall_sem_wait(MUTEX_ID);
    states[id] = HUNGRY;
    test(id);
    syscall_sem_post(MUTEX_ID);
    syscall_sem_wait(pids[id]);
}

static void put_forks(int id) {
    syscall_sem_wait(MUTEX_ID);
    states[id] = THINKING;
    test(LEFT(id));
    test(RIGHT(id));
    syscall_sem_post(MUTEX_ID);
}

static void test(int id) {
    if (states[id] == HUNGRY && states[LEFT(id)] != EATING && states[RIGHT(id)] != EATING) {
        states[id] = EATING;
        syscall_sem_post(pids[id]);
    }
}