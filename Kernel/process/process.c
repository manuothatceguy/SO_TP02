#include <process.h>
#include <pcb.h>
#include <memoryManager.h>

// lista doblemente encadenada circular
typedef struct ProcessNode {
    PCB *process;
    struct ProcessNode *next;
    struct ProcessNode *prev;
} ProcessNode;

typedef struct ProcessList {
    ProcessNode* current; // puntero al nodo cabeza
    uint64_t numProcesses; // cantidad de procesos en la lista
} ProcessList;

ProcessLinkedPtr createProcessLinkedList(){
    ProcessLinkedPtr list = (ProcessLinkedPtr)allocMemory(sizeof(ProcessList));
    if (list == NULL) {
        return NULL; 
    }
    list->current = NULL;
    list->numProcesses = 0;
    return list;
}

void addProcess(ProcessLinkedPtr list, PCB *process){
    if (list == NULL || process == NULL) {
        return;
    }
    ProcessNode *newNode = (ProcessNode *)allocMemory(sizeof(ProcessNode));
    if (newNode == NULL) {
        return;
    }
    newNode->process = process;
    newNode->next = NULL;
    newNode->prev = NULL;
    if (list->current == NULL) {
        list->current = newNode;
        list->current->next = newNode;
        list->current->prev = newNode;
    } else { // la política es: agrego al final, siempre.
        list->current->prev->next = newNode;
        newNode->prev = list->current->prev;
        newNode->next = list->current;
        list->current->prev = newNode;
    }
}

void removeProcess(ProcessLinkedPtr list, PCB *process){
    if (list == NULL || process == NULL) {
        return;
    }
    ProcessNode *current = list->current;
    while (current != NULL) {
        if (current->process == process) {
            current->prev->next = current->next;
            current->next->prev = current->prev;
            freeMemory(current);
            return;
        }
        current = current->next;
    }
}

void freeProcessLinkedList(ProcessLinkedPtr list){
    if (list == NULL) {
        return;
    }
    ProcessNode *current = list->current;
    while (current != NULL) {
        ProcessNode *temp = current;
        current = current->next;
        freeMemory(temp);
    }
    freeMemory(list);
}

PCB* getProcess(ProcessLinkedPtr list, pid_t pid){
    if (list == NULL) {
        return NULL;
    }
    ProcessNode *current = list->current;
    while (current != NULL) {
        if (current->process->pid == pid) {
            return current->process;
        }
        current = current->next;
    }
    return NULL;
}

PCB* getNextProcess(ProcessLinkedPtr list){
    if (list == NULL || list->current == NULL) {
        return NULL;
    }

    ProcessNode* start = list->current;
    ProcessNode* current = list->current->next;
    
    // Buscar el siguiente proceso listo que no sea idle
    while (current != start) {
        if (current->process->state == READY && current->process->pid != 0) {
            list->current = current;
            return current->process;
        }
        current = current->next;
    }
    
    // Si no hay otros procesos listos, retornar idle
    if (start->process->pid == 0) {
        list->current = start;
        return start->process;
    }
    
    return NULL;
}

PCB* getCurrentProcess(ProcessLinkedPtr list){
    if (list == NULL) {
        return NULL;
    }
    if (list->current == NULL) {
        return NULL;
    }
    return list->current->process;
}