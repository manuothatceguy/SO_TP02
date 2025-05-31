#include <process.h>
//#include <pcb.h>
#include <shared_structs.h>
#include <memoryManager.h>
#include <debug.h>
#include <queue.h>

typedef struct ProcessManagerCDT {
    PCB* currentProcess;
    QueueADT readyQueue;
    QueueADT blockedQueue;
} ProcessManagerCDT;

ProcessManagerADT createProcessManager(){
    ProcessManagerADT list = (ProcessManagerADT)allocMemory(sizeof(ProcessManagerCDT));
    if (list == NULL) {
        return NULL; 
    }
    list->currentProcess = NULL;
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
    return list;
}

void addProcess(ProcessManagerADT list, PCB *process){
    if(list == NULL || process == NULL) {
        return;
    }
    enqueue(list->readyQueue, process);
    if(list->currentProcess == NULL) {
        list->currentProcess = process; 
    }
}

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

void removeProcess(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return;
    }
    removeFromQueue(list->readyQueue, &pid, hasPid);
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
    
    if (list->currentProcess && list->currentProcess->pid == pid) {
        list->currentProcess = NULL; 
    }
    
    return 0;
}

int unblockProcessQueue(ProcessManagerADT list, pid_t pid) {
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
    return (PCB*)containsQueue(list->blockedQueue, &pid, hasPid);
}

PCB* getNextProcess(ProcessManagerADT list){
    if(list == NULL || isQueueEmpty(list->readyQueue)) {
        return NULL;
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

uint64_t getProcessCount(ProcessManagerADT list) {
    return list ? queueSize(list->readyQueue) + queueSize(list->blockedQueue) : 0;
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