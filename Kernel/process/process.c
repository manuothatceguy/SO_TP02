#include <process.h>
//#include <pcb.h>
#include <shared_structs.h>
#include <memoryManager.h>
#include <debug.h>
#include <queue.h>
#include <pipes.h>

typedef struct ProcessManagerCDT {
    PCB* foregroundProcess;
    PCB* currentProcess;
    QueueADT readyQueue;
    QueueADT blockedQueue;
    QueueADT blockedQueueBySem;
    QueueADT zombieQueue;
    PCB* idleProcess;
} ProcessManagerCDT;

int compareProcesses(void* a, void* b) {
    PCB* processA = (PCB*)a;
    PCB* processB = (PCB*)b;
    return (processA->pid == processB->pid) ? 0 : -1;
}

int hasPid(void* a, void*b){ // a es el proceso, b es el pid. el signature es así por temas de casteo...
    PCB* processA = (PCB*)a;
    pid_t* pid = (pid_t*)b;
    return (processA->pid == *pid) ? 0 : -1; 
}

ProcessManagerADT createProcessManager(){
    ProcessManagerADT list = (ProcessManagerADT)allocMemory(sizeof(ProcessManagerCDT));
    if (list == NULL) {
        return NULL; 
    }
    list->foregroundProcess = NULL;
    list->currentProcess = NULL;
    list->idleProcess = NULL;
    list->readyQueue = createQueue();
    if (list->readyQueue == NULL) {
        freeMemory(list);
        return NULL; 
    }
    list->blockedQueue = createQueue();
    if (list->blockedQueue == NULL) {
        freeQueue(list->readyQueue);
        freeMemory(list);
        return NULL; 
    }
    list->zombieQueue = createQueue();
    if (list->zombieQueue == NULL) {
        freeQueue(list->readyQueue);
        freeQueue(list->blockedQueue);
        freeMemory(list);
        return NULL; 
    }
    list->blockedQueueBySem = createQueue();
    if (list->blockedQueueBySem == NULL) {
        freeQueue(list->readyQueue);
        freeQueue(list->blockedQueue);
        freeQueue(list->zombieQueue);
        freeMemory(list);
        return NULL; 
    }
    return list;
}

void addProcess(ProcessManagerADT list, PCB *process, char foreground){
    if(list == NULL || process == NULL) {
        return;
    }
    if(list->currentProcess == list->idleProcess) {
        list->currentProcess = process; // Set the first process as current
    }
    if(foreground) {
        // If there's a current foreground process, block it and make it wait for the new one
        if(list->foregroundProcess != NULL) {
            PCB* oldForeground = list->foregroundProcess;
            // Remove from current queue if it exists
            removeFromQueue(list->readyQueue, &oldForeground->pid, hasPid);
            removeFromQueue(list->blockedQueue, &oldForeground->pid, hasPid);
            removeFromQueue(list->blockedQueueBySem, &oldForeground->pid, hasPid);
            // Add to blocked queue and set it to wait for the new process
            enqueue(list->blockedQueue, oldForeground);
            oldForeground->state = BLOCKED;
            oldForeground->waitingForPid = process->pid;
        }
        // Set new foreground process
        list->foregroundProcess = process;
        process->state = READY;
        
        // For foreground processes, use the same stdin as the shell
        if (process->pid != 0) {  // If not the shell
            process->fds.stdin = 0;  // Use pipe 0 (shell's stdin)
            // Create stdout pipe only for non-shell foreground processes
            if (process->fds.stdout == -1) {
                int stdout_fd = createPipe();
                if(stdout_fd >= 0) {
                    process->fds.stdout = stdout_fd;
                }
            }
        }
    }
    enqueue(list->readyQueue, process);
}

void removeProcess(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return;
    }
    removeFromQueue(list->readyQueue, &pid, hasPid);
    if(list->foregroundProcess && list->foregroundProcess->pid == pid) {
        list->foregroundProcess = NULL;
    }
}

void removeZombieProcess(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return;
    }
    removeFromQueue(list->zombieQueue, &pid, hasPid);
}

static PCB* switchProcess(QueueADT qFrom, QueueADT qTo, pid_t pid) {
    if (qFrom == NULL || qTo == NULL) {
        return NULL;
    }
    PCB* process = (PCB*)removeFromQueue(qFrom, &pid, hasPid);
    if (process == NULL) {
        return NULL;
    }
    if (enqueue(qTo, process) != 0) {
        enqueue(qFrom, process);
        return NULL;
    }
    return process;
}

int blockProcessQueue(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    PCB* process = switchProcess(list->readyQueue, list->blockedQueue, pid);
    if (process == NULL) {
        return -1;
    }
    process->state = BLOCKED;
    /* ROMPE EL getCurrentPid() siendo -1 y no ejecuta el yield()
        volviendo a la ejecucion como si no estuviera bloqueado!

        
    if (list->currentProcess && list->currentProcess->pid == pid) {
        list->currentProcess = NULL; 
    }
    */
    return 0;
}

int blockProcessQueueBySem(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    PCB* process = switchProcess(list->readyQueue, list->blockedQueueBySem, pid);
    if (process == NULL) {
        return -1;
    }
    process->state = WAITING_SEM;
    return 0;
}

int unblockProcessQueue(ProcessManagerADT list, pid_t pid , ProcessState state) {
    if (list == NULL) {
        return -1; 
    }
    PCB* process = switchProcess(list->blockedQueue, list->readyQueue, pid);
    if (process == NULL) {
        return -1;
    }
    process->state = READY; 
    return 0;
}

int unblockProcessQueueBySem(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    PCB* process = switchProcess(list->blockedQueueBySem, list->readyQueue, pid);
    if (process == NULL) {
        return -1;
    }
    process->state = READY; 
    return 0;
}

void freeProcessLinkedList(ProcessManagerADT list){
    if (list == NULL) {
        return;
    }
    freeQueue(list->readyQueue);
    freeQueue(list->blockedQueue);
    freeMemory(list);
}

PCB* getProcess(ProcessManagerADT list, pid_t pid){
    if (list == NULL) {
        return NULL;
    }
    PCB* process = (PCB*)containsQueue(list->readyQueue, &pid, hasPid);
    if (process != NULL) {
        return process; // Found in ready queue
    }
    process = (PCB*)containsQueue(list->blockedQueue, &pid, hasPid);
    if (process != NULL) {
        return process; // Found in blocked queue
    }
    return (PCB*)containsQueue(list->zombieQueue, &pid, hasPid);
}

PCB* getNextProcess(ProcessManagerADT list){
    if(list == NULL) {
        return NULL;   
    }

    if(isQueueEmpty(list->readyQueue)) {
        list->currentProcess = list->idleProcess;
        return list->idleProcess;
    }
    
    PCB* nextProcess = (PCB*)dequeue(list->readyQueue);
    if (nextProcess == NULL) {
        return NULL; 
    }

    if(enqueue(list->readyQueue, nextProcess) != 0) {
        return NULL;
    }
    
    list->currentProcess = nextProcess;
    return nextProcess;
}

int hasNextReady(ProcessManagerADT list) {
    return !isQueueEmpty(list->readyQueue);
}

PCB* getCurrentProcess(ProcessManagerADT list){
    return list ? list->currentProcess : NULL;
}

void setIdleProcess(ProcessManagerADT list, PCB* idleProcess) {
    if (list != NULL){
        list->idleProcess = idleProcess; 
    }
}

PCB* getIdleProcess(ProcessManagerADT list) {
    return list ? list->idleProcess : NULL;
}

uint64_t getProcessCount(ProcessManagerADT list) {
    return list ? queueSize(list->readyQueue) + queueSize(list->blockedQueue) + queueSize(list->zombieQueue) : 0;
}

uint64_t countReadyProcesses(ProcessManagerADT list) {
    return list ? queueSize(list->readyQueue) : 0;
}
uint64_t countBlockedProcesses(ProcessManagerADT list) {
    return list ? queueSize(list->blockedQueue) : 0;
}
uint64_t countProcesses(ProcessManagerADT list) {
    return list ? getProcessCount(list) : 0;
}

PCB* getForegroundProcess(ProcessManagerADT list) {
    return list ? list->foregroundProcess : NULL;
}

char isForegroundProcess(ProcessManagerADT list, pid_t pid) {
    return list && list->foregroundProcess && list->foregroundProcess->pid == pid;
}

PCB* killProcess(ProcessManagerADT list, pid_t pid, uint64_t retValue, ProcessState state) {
    if (list == NULL) {
        return NULL; 
    }
    
    PCB* process = switchProcess(list->readyQueue, list->zombieQueue, pid);
    if (process == NULL) {
        process = switchProcess(list->blockedQueue, list->zombieQueue, pid);
        if (process == NULL) {
            return NULL; 
        }
    }

    // If the killed process was the foreground process, clear it
    if (list->foregroundProcess && list->foregroundProcess->pid == pid) {
        list->foregroundProcess = NULL;
    }

    process->retValue = retValue;
    process->state = state;
    return process;
}

// Add new function to get process I/O
int getProcessStdin(ProcessManagerADT list, pid_t pid) {
    PCB* process = getProcess(list, pid);
    return process ? process->fds.stdin : -1;
}

int getProcessStdout(ProcessManagerADT list, pid_t pid) {
    PCB* process = getProcess(list, pid);
    return process ? process->fds.stdout : -1;
}