#include <process.h>
//#include <pcb.h>
#include <shared_structs.h>
#include <memoryManager.h>
#include <debug.h>

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
    newNode->next = list->current;
    if (list->current == NULL) {
        list->current = newNode;
        list->current->next = newNode;
        list->current->prev = newNode;
    } else { // la política es: agrego al final, siempre.
        list->current->prev->next = newNode;
        newNode->prev = list->current->prev;
        list->current->prev = newNode;
    }
    list->numProcesses++;
}

void removeProcess(ProcessLinkedPtr list, pid_t pid) {
    if (list == NULL || list->current == NULL) {
        return;
    }
    
    ProcessNode *start = list->current;
    ProcessNode *current = start;
    
    do {
        if (current->process->pid == pid) {
            // Manejo especial si es el único nodo
            if (current->next == current) {
                list->current = NULL;
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                if (list->current == current) {
                    list->current = current->next;
                }
            }
            
            freeMemory(current);
            list->numProcesses--;
            return;
        }
        
        current = current->next;
    } while (current != start);
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
    if (list == NULL || list->current == NULL || list->numProcesses == 0) {
        return NULL;
    }
    ProcessNode* current = list->current;
    list->current = current->next;
    return list->current->process;
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

uint64_t getProcessCount(ProcessLinkedPtr list) {
    if (list == NULL) {
        return 0;
    }
    return list->numProcesses;
}