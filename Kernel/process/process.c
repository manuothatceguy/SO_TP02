// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <process.h>
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
    if (a == NULL || b == NULL) {
        return -1;
    }
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

void addProcess(ProcessManagerADT list, PCB *process){
    if(list == NULL || process == NULL) {
        return;
    }

    if(list->currentProcess == NULL || list->currentProcess == list->idleProcess) {
        list->currentProcess = process; 
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
    QueueADT queue = list->readyQueue;
    PCB* process = (PCB*)containsQueue(list->readyQueue, &pid, hasPid);
    if (process == NULL) {
        process = (PCB*)containsQueue(list->blockedQueueBySem, &pid, hasPid);
        if(process == NULL) {
            return -1; 
        }
        queue = list->blockedQueueBySem;
    }
    
    process = switchProcess(queue, list->blockedQueue, pid);
    if (process == NULL) {
        return -1;
    }
    
    if (process->state != BLOCKED) {
        process->state = BLOCKED;
    }
    
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
    
    if (list->currentProcess && list->currentProcess->pid == pid) {
        return list->currentProcess;
    }
    
    if (list->idleProcess && list->idleProcess->pid == pid) {
        return list->idleProcess;
    }
    
    PCB* process = (PCB*)containsQueue(list->readyQueue, &pid, hasPid);
    if (process != NULL) {
        return process; 
    }
    
    process = (PCB*)containsQueue(list->blockedQueue, &pid, hasPid);
    if (process != NULL) {
        return process; 
    }
    
    process = (PCB*)containsQueue(list->blockedQueueBySem, &pid, hasPid);
    if (process != NULL) {
        return process; 
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
    if(isForegroundProcess(nextProcess)) {
        setForegroundProcess(list, nextProcess);
    } 
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
        list->currentProcess = idleProcess;
    }
}

void setForegroundProcess(ProcessManagerADT list, PCB* foregroundProcess) {
    if (list != NULL){
        list->foregroundProcess = foregroundProcess; 
    }
}

PCB* getIdleProcess(ProcessManagerADT list) {
    return list ? list->idleProcess : NULL;
}

uint64_t getProcessCount(ProcessManagerADT list) {
    return list ? queueSize(list->readyQueue) + queueSize(list->blockedQueue) + queueSize(list->zombieQueue) + queueSize(list->blockedQueueBySem) + 1 : 0; //counts idle
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

char isCurrentForegroundProcess(ProcessManagerADT list, pid_t pid) {
    return list && list->foregroundProcess && list->foregroundProcess->pid == pid;
}

char isForegroundProcess(PCB* process) {
    return process && process->foreground == 1;
}

void adjustHierarchy(QueueADT queue, pid_t oldPid, pid_t newPid, char shouldRemove) {
    if(queue == NULL || isQueueEmpty(queue)) {
        return; 
    }
    PCB** processes = (PCB**)dumpQueue(queue);
    if (processes == NULL) {
        return; 
    }
    for (int i = 0; processes[i] != NULL; i++) {
        PCB* process = processes[i];
        if (process->parentPid == oldPid) {
            if(shouldRemove)
                removeFromQueue(queue, &process->pid, hasPid);
            else
                process->parentPid = newPid; 
        } 
    }
    freeMemory(processes); 
}

PCB* killProcess(ProcessManagerADT list, pid_t pid, uint64_t retValue, ProcessState state) {
    if (list == NULL) {
        return NULL; 
    }
    
    PCB* process = switchProcess(list->readyQueue, list->zombieQueue, pid);
    if (process == NULL && (retValue == 1 || retValue == 9) ) {
        process = switchProcess(list->blockedQueue, list->zombieQueue, pid);
        if (process == NULL) {
            process = switchProcess(list->blockedQueueBySem, list->zombieQueue, pid);
            if (process == NULL) {
                return NULL; 
            }
        }
    } else if (process == NULL) {
        return NULL;
    }

    if (list->foregroundProcess && list->foregroundProcess->pid == pid) {
        list->foregroundProcess = NULL;
    }

    process->retValue = retValue;
    process->state = state;
    adjustHierarchy(list->readyQueue, pid, list->idleProcess->pid, 0);
    adjustHierarchy(list->blockedQueue, pid, list->idleProcess->pid, 0);
    adjustHierarchy(list->blockedQueueBySem, pid, list->idleProcess->pid, 0);
    adjustHierarchy(list->zombieQueue, pid, list->idleProcess->pid, 1);
    return process;
}

int getProcessStdin(ProcessManagerADT list, pid_t pid) {
    PCB* process = getProcess(list, pid);
    return process ? process->fds.stdin : -1;
}

int getProcessStdout(ProcessManagerADT list, pid_t pid) {
    PCB* process = getProcess(list, pid);
    return process ? process->fds.stdout : -1;
}

int isInAnyQueue(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) return 0;
    
    if (containsQueue(list->readyQueue, &pid, hasPid) != NULL) {
        return 1;
    }
    
    if (containsQueue(list->blockedQueue, &pid, hasPid) != NULL) {
        return 1;
    }
    
    if (containsQueue(list->blockedQueueBySem, &pid, hasPid) != NULL) {
        return 1;
    }
    
    if (containsQueue(list->zombieQueue, &pid, hasPid) != NULL) {
        return 1;
    }
    
    return 0;
}

void addToReadyQueue(ProcessManagerADT list, PCB* process) {
    if (list == NULL || process == NULL) return;
    enqueue(list->readyQueue, process);
}

void addToBlockedQueue(ProcessManagerADT list, PCB* process) {
    if (list == NULL || process == NULL) return;
    enqueue(list->blockedQueue, process);
}

void addToBlockedQueueBySem(ProcessManagerADT list, PCB* process) {
    if (list == NULL || process == NULL) return;
    enqueue(list->blockedQueueBySem, process);
}