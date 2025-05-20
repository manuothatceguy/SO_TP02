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

    // Guarda el proceso actual
    ProcessNode* start = list->current;
    
    // Primero, intenta avanzar al siguiente proceso
    ProcessNode* current = list->current->next;
    
    // Da una vuelta completa
    do {
        if (current->process->state == READY && current->process->pid != 0) {
            list->current = current;
            return current->process;
        }
        current = current->next;
    } while (current != start);
    
    // Si volvimos al principio, comprobamos el proceso actual
    if (start->process->state == READY) {
        return start->process;  // Mantenemos el mismo proceso
    }
    
    // Si todo falla, intentamos con idle
    current = list->current;
    do {
        if (current->process->pid == 0 && current->process->state == READY) {
            list->current = current;
            return current->process;
        }
        current = current->next;
    } while (current != start);
    
    return NULL;  // No hay nada que ejecutar
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