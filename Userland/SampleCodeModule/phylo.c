#include <syscall.h>
#include <stdlib.h>
#include <phylo.h>

#define MAX_PHYLOS 7
#define MIN_PHYLOS 3

#define MUTEX_ID 100

#define TIME 1
#define philosopherSemaphore(i) (MUTEX_ID + (i) + 1)

#define LEFT(phyloId) (((phyloId) + phylosCount - 1) % phylosCount)
#define RIGHT(phyloId) (((phyloId) + 1) % phylosCount)

typedef enum { THINKING, HUNGRY, EATING } state_t;

int phylosCount = 0;

state_t state[MAX_PHYLOS] = {0};
int philosopherPids[MAX_PHYLOS] = {0};

static void printState() {
	printf("\n");
	for (int i = 0; i < phylosCount; i++) {
		printf(state[i] == EATING ? "E " : ". ");
	}
	printf("\n");
}

static void test(int phyloId) {
	if (state[phyloId] == HUNGRY && state[LEFT(phyloId)] != EATING && state[RIGHT(phyloId)] != EATING) {
		state[phyloId] = EATING;
        printState();
		syscall_sem_post(philosopherSemaphore(phyloId));
	}
}

static void takeForks(int phyloId) {
	syscall_sem_wait(MUTEX_ID);
	state[phyloId] = HUNGRY;
	test(phyloId);
	syscall_sem_post(MUTEX_ID);
	syscall_sem_wait(philosopherSemaphore(phyloId));
}

static void putForks(int phyloId) {
	syscall_sem_wait(MUTEX_ID);
	state[phyloId] = THINKING;
	test(LEFT(phyloId));
	test(RIGHT(phyloId));
	syscall_sem_post(MUTEX_ID);
}

static uint64_t philosopher(uint64_t argc, char *argv[]){
	int i = atoi(argv[0]);
    state[i] = THINKING;

	while (1) {
		syscall_wait(TIME);
		takeForks(i);
		syscall_wait(TIME);
		putForks(i);
	}
	return 0;
}

static void addPhilosopher() {
	syscall_sem_wait(MUTEX_ID);
	state[phylosCount] = THINKING;

	if (syscall_sem_open(philosopherSemaphore(phylosCount) , 0) == -1) {
		printf("Error al crear semáforo %d\n", philosopherSemaphore(phylosCount));
		return;
	}

    char *id_str = (char *)syscall_allocMemory(2);
    if (id_str == NULL) {
        syscall_freeMemory(id_str);
        return;
    }
    intToStr(phylosCount, id_str);

    char **argv = (char **)syscall_allocMemory(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        syscall_freeMemory(argv);
        return;
    }

    argv[0] = id_str;
    argv[1] = NULL;
        
	philosopherPids[phylosCount] = syscall_create_process("philosopher", philosopher, 1, argv, 1, 0, 0, 1);
	if (philosopherPids[phylosCount] < 0) {
		printf("Error al crear filosofo %d\n", phylosCount);
		return;
	}

	phylosCount++;
	syscall_sem_post(MUTEX_ID);
}

static void removePhilosopher() {
	phylosCount--;

    printf("Eliminando filosofo...\n");

	syscall_sem_wait(MUTEX_ID);
	while (state[LEFT(phylosCount)] == EATING && state[RIGHT(phylosCount)] == EATING) {
		syscall_sem_post(MUTEX_ID);
		syscall_sem_wait(philosopherSemaphore(phylosCount));
		syscall_sem_wait(MUTEX_ID);
	}
	if (syscall_kill(philosopherPids[phylosCount]) == -1) {
		printferror("Error al matar filosofo %d\n", phylosCount);
		return;
	}
    syscall_waitpid(philosopherPids[phylosCount], NULL);
	//printf("matado ok");
	if (syscall_sem_close(philosopherSemaphore(phylosCount)) == -1) {
		printferror("Error al cerrar semaforo %d\n", philosopherSemaphore(phylosCount));
		return;
	}
	//printf("sem cerrado ok");
	//printf("eliminado ok");
	syscall_sem_post(MUTEX_ID);
}
void start() {
	char c;
	while (1) {
		c = getChar();
		if(c == 'a') {
			if(phylosCount < MAX_PHYLOS) {
                printf("Agregando filosofo\n");
                addPhilosopher();
			}
			else {
                printf("No se pueden agregar mas filosofos\n");
            }
		} else if (c == 'e') {
            if (phylosCount > MIN_PHYLOS) {
                removePhilosopher();
            } else {
                printf("No se pueden eliminar mas filosofos\n");
            }
		} else if (c == 'f') {
            break;
		}
	}

	for (int i = 0; i < phylosCount; i++) {
        printf("Esperando a filosofo %d\n", i);
		if (syscall_kill(philosopherPids[i]) == -1) {
			printferror("Error al matar filosofo %d\n", i);
			return;
		}
        syscall_waitpid(philosopherPids[i], NULL);
        printf("Filosofo %d terminado\n", i);
		if (syscall_sem_close(philosopherSemaphore(i)) == -1) {
			printferror("Error al cerrar semaforo %d\n", i);
			return;
		}
	}
    printf("Todos los filosofos han terminado\n");
	if (syscall_sem_close(MUTEX_ID) == -1) {
		printferror("Error al cerrar mutex\n");
		return;
	}
	return;
}

uint64_t phylo(uint64_t argc, char **argv) {
	uint64_t aux = atoi(argv[0]);
	if (aux < MIN_PHYLOS || aux > MAX_PHYLOS) {
		printf("Cantidad de filosofos debe estar entre %d y %d\n", MIN_PHYLOS, MAX_PHYLOS);
		return;
	}
	if (syscall_sem_open(MUTEX_ID, 1) == -1) {
		printf("Error al crear semaforo mutex\n");
		return;
	}

	printf("\nFilosofos comensales\n");
	printf("Comandos: 'a' agregar, 'e' eliminar, 'f' finalizar\n");
	printf("Comenzamos...\n");
	for (int i = 0; i < aux; i++) {
		addPhilosopher();
	}

	start();
    
	phylosCount = 0;
	return 0;
}