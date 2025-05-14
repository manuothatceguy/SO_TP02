#include <scheduler.h>
#include <processLinkedList.h>
#include <defs.h>
#include <memoryManager.h>
#include <pcb.h>
#include <string.h>

#define QUANTUM 10

static ProcessLinkedPtr processes = NULL;    
static uint64_t processCount = 0;           
static pid_t currentPid = -1;   
static pid_t nextFreePid = 0;
static uint64_t quantum = 0;            


void initScheduler(ProcessLinkedPtr list) {
    processes = list;
}

pid_t createProcess(char* name, void(*function)(void*),uint64_t argc, char **arg, uint8_t priority) {
    PCB* process = allocMemory(sizeof(PCB));
    if (process == NULL) {
        return -1; 
    }

    strncpy(process->name, name, NAME_MAX_LENGTH);
    process->pid = nextFreePid++;
    process->parentPid = currentPid;
    process->state = READY;
    process->priority = priority;
    process->base = (uint64_t)allocMemory(STACK_SIZE);

    if ((void *)process->base == NULL) {
        freeMemory(process); 
        return -1;
    }
    process->base += STACK_SIZE;

    process->rip = (uint64_t)function;
    // !! process->rsp = processStackFrame(process->base, process->rip , argc, arg); // HACER 
    // processStackFrame guarda los valores de los registros en la pila, ver si lo hacemos asi o 
    // guardamos los registros en el struct Registers

    addProcess(processes, process);
    processCount++;
    return process->pid;
}

pid_t getCurrentPid() {
    return currentPid;
}

uint64_t schedule(uint64_t rsp){
    return rsp; // por ahora no hace nada
    /*
    if(processList == NULL) {
        return rsp; // No hay procesos para programar
    }
    
    if (quantum == 0) {
        PCB* currentProcess = getNextProcess(processList);
        quantum = QUANTUM; // dsp ver de calcular segun la prioridad
        return currentProcess->registers->rsp;
    }
    // Si hay tiempo restante, se reduce y se devuelve el rsp actual
    quantum--;
    return rsp;
    */
}