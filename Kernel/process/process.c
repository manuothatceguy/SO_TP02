#include <process.h>
#include <pcb.h>
#include <memoryManager.h>
#include <lib.h>
 
static pid_t nextFreePid = 0; 

// lista doblemente encadenada circular
typedef struct ProcessNode {
    PCB *process;
    struct ProcessNode *next;
    struct ProcessNode *prev;
} ProcessNode;

typedef struct ProcessList {
    PCB* running; // puntero al nodo cabeza
    ProcessNode *ready;
    ProcessNode *blocked;
} ProcessList;

ProcessListADT createProcessLinkedList(){
    ProcessListADT list = (ProcessListADT)allocMemory(sizeof(ProcessList));
    if (list == NULL) {
        return NULL; 
    }
    list->running = NULL;
    list->ready = NULL;
    list->blocked = NULL;
    return list;
}

static inline ProcessNode* insertIntoList(ProcessNode *list, ProcessNode *newNode) {
    if (newNode == NULL) {
        return list;
    }
    if(list == NULL) {
        newNode->next = newNode;
        newNode->prev = newNode;
        list = newNode;
    }
    newNode->next = list->next;
    newNode->prev = list;
    list->next->prev = newNode;
    list->next = newNode;
    return list;
}

static inline ProcessNode* pickFromList(ProcessNode* list, pid_t pid, ProcessNode** returnNode) {
    if (list == NULL) {
        return NULL;
    }
    *returnNode = NULL;
    if(pid == -2){ // pick first
        *returnNode = list;
        if (list->next == list) { // tengo un solo elemento
            list->next = list->prev = NULL;
            return NULL;
        }
        list->prev->next = list->next;
        list->next->prev = list->prev;
        return list;
    }
    ProcessNode *current = list;
    do {
        if (current->process->pid == pid) {
            if (current->next == current) { // tengo un solo elemento
                current->next = current->prev = NULL;
                *returnNode = current;
                return NULL;
            }
            current->prev->next = current->next;
            current->next->prev = current->prev;
            *returnNode = current;
            return list;
        }
        current = current->next;
    } while (current != list->next);
    return list; // not found
}

extern void wrapper();

static inline uint64_t countArgs(char **argv) {
    uint64_t count = 0;
    while (argv[count] != NULL) {
        count++;
    }
    return count;
}

pid_t createReadyProcess(ProcessListADT list, char* name, fnptr function, char **argv, uint8_t priority) {
    if (name == NULL || function == NULL) {
        return -1;
    }

    uint64_t argc = countArgs(argv);

    PCB* newProcess = allocMemory(sizeof(PCB));
    if (newProcess == NULL) {
        return -1; 
    }

    if(priority < IDLE_PRIORITY){
        priority = IDLE_PRIORITY;
    } else if(priority > MAX_PRIORITY){
        priority = MAX_PRIORITY;
    }

    strncpy(newProcess->name, name, NAME_MAX_LENGTH);
    newProcess->pid = nextFreePid++;
    newProcess->parentPid = list->running ? list->running->pid : 0;
    newProcess->priority = priority;
    newProcess->base = (uint64_t)allocMemory(STACK_SIZE);

    if ((void *)newProcess->base == NULL) {
        freeMemory(newProcess);
        return -1;
    }

    newProcess->base += STACK_SIZE;
    newProcess->rsp = processStackFrame(newProcess->base + STACK_SIZE, (uint64_t)function, argc, argv);
    ProcessNode *newProcessNode = (ProcessNode *)allocMemory(sizeof(ProcessNode));
    if (newProcessNode == NULL) {
        freeMemory((void*)newProcess->base);
        freeMemory(newProcess);
        return -1; 
    }
    newProcessNode->process = newProcess;
    list->ready = insertIntoList(list->ready, newProcessNode);
    return newProcess->pid;
}

pid_t readyToBlocked(ProcessListADT list, pid_t pid){
    if (list == NULL) {
        return -1;
    }
    ProcessNode *blockedNode;
    list->ready = pickFromList(list->ready, pid, &blockedNode);
    if (blockedNode == NULL) {
        return -1; // no se encontro el proceso
    }
    list->blocked = insertIntoList(list->blocked, blockedNode);
    return pid;
}

pid_t blockedToReady(ProcessListADT list, pid_t pid){
    if (list == NULL) {
        return -1;
    }
    ProcessNode *unblockedNode;
    list->blocked = pickFromList(list->blocked, pid, &unblockedNode);
    if (unblockedNode == NULL) {
        return -1; // no se encontro el proceso
    }
    list->ready = insertIntoList(list->ready, unblockedNode);
    return pid;
}

PCB* getRunningProcess(ProcessListADT list){
    return list ? list->running : NULL;
}

PCB* getNextProcess(ProcessListADT list){
    ProcessNode* nextProcess;
    list->ready = pickFromList(list->ready, -2, &nextProcess);
    list->running = nextProcess->process;
    return nextProcess->process;
}

static inline PCB* getProcess(ProcessNode* list, pid_t pid){
    if (list == NULL) {
        return NULL;
    }
    if(pid == -2){ // pick first
        return list->process;
    }
    ProcessNode *current = list;
    do {
        if (current->process->pid == pid) {
            if (current->next == current) { // tengo un solo elemento
                return current->process;
            }
            current->prev->next = current->next;
            current->next->prev = current->prev;
            return current->process;
        }
        current = current->next;
    } while (current != list->next);
    return NULL;
}

int8_t changePriority(ProcessListADT list, pid_t pid, uint8_t newPrio){
    if (pid == -1) {
        return -1;
    }
    if (newPrio < MIN_PRIORITY) {
        newPrio = MIN_PRIORITY; 
    } else if (newPrio > MAX_PRIORITY) {
        newPrio = MAX_PRIORITY; 
    }
    PCB *processNode = getProcess(list->ready, pid);
    processNode = processNode ? processNode : getProcess(list->blocked, pid);
    if (processNode == NULL) {
        return -1; // no se encontro el proceso
    }
    processNode->priority = newPrio;
    return 0;
}