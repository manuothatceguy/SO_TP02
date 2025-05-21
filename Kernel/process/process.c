#include <process.h>
#include <pcb.h>
#include <memoryManager.h>
 
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

static inline ProcessList* insertIntoList(ProcessNode *list, ProcessNode *newNode) {
    if (newNode == NULL) {
        return;
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
}


/**
 * si tengo | x | -> | y | -> | z | en la lista y quiero "y"
 * entonces retorno | y | y me queda | x | -> | z | en la lista
 */
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

    PCB* process = allocMemory(sizeof(PCB));
    if (process == NULL) {
        return -1; 
    }

    if(priority < IDLE_PRIORITY){
        priority = IDLE_PRIORITY;
    } else if(priority > MAX_PRIORITY){
        priority = MAX_PRIORITY;
    }

    strncpy(process->name, name, NAME_MAX_LENGTH);
    process->pid = nextFreePid++;
    process->parentPid = list->running ? list->running->pid : 0;
    process->priority = priority;
    process->base = (uint64_t)allocMemory(STACK_SIZE);

    if ((void *)process->base == NULL) {
        freeMemory(process); 
        return -1;
    }

    process->base += STACK_SIZE;
    process->rsp = processStackFrame(process->base + STACK_SIZE, (uint64_t)function, argc, (uint64_t)argv);
    list->ready = insertIntoList(list->ready, process);
    return process->pid;
}

pid_t readyToBlocked(ProcessListADT list, pid_t pid){
    if (list == NULL) {
        return -1;
    }
    ProcessNode *blockedNode;
    list = pickFromList(list->ready, pid, &blockedNode);
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
    list = pickFromList(list->blocked, pid, &unblockedNode);
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
    PCB* nextProcess;
    list->ready = pickFromList(list->ready, -2, &nextProcess);
    list->running = nextProcess;
    return nextProcess;
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
    ProcessNode *processNode;
    processNode = getProcess(list->ready, pid) ? processNode : getProcess(list->blocked, pid);
    if (processNode == NULL) {
        return -1; // no se encontro el proceso
    }
    processNode->process->priority = newPrio;
    return 0;
}