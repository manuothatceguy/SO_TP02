#include <scheduler.h>
#include <defs.h>
#include <memoryManager.h>
#include <interrupts.h>
//#include <pcb.h>
#include <shared_structs.h>
#include <lib.h>
#include <textModule.h>
#include <syscall.h>
#include <debug.h>

#define QUANTUM 3
#define MAX_PRIORITY 5 // agregar a limitaciones
#define MIN_PRIORITY 0
#define IDLE_PRIORITY -1

static char first = 1;

uint64_t calculateQuantum(uint8_t priority) {
    if (priority == IDLE_PRIORITY) {
        return QUANTUM;  // Minimum quantum for idle
    }
    return QUANTUM * (MAX_PRIORITY - priority + 1);
}

static ProcessManagerADT processes = NULL;              
static pid_t currentPid = -1;   
static pid_t nextFreePid = 0; // 0 es el idle!
//static uint64_t readyProcesses = 0; // cantidad de procesos listos
static uint64_t quantum = 0;    

void initScheduler(ProcessManagerADT list) {
    processes = list;
}

pid_t createProcess(char* name, fnptr function, uint64_t argc, char **arg, uint8_t priority) {
    if (name == NULL || function == NULL) {
        return -1;
    }

    if (argc > 0 && arg == NULL) {
        return -1;
    }

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

    if (currentPid == -1) {
        process->parentPid = -1; 
    } else {
        process->parentPid = currentPid;
    }

    process->state = READY;
    //readyProcesses++;
    
    process->priority = priority;
    process->base = (uint64_t)allocMemory(STACK_SIZE);

    if ((void *)process->base == NULL) {
        freeMemory(process); 
        return -1;
    }
    process->base += STACK_SIZE;
    process->rip = (uint64_t)function;
    process->rsp = processStackFrame(process->base, (uint64_t)function, argc, arg);
    addProcess(processes, process);
    DEBUG_PRINT("Creating process...\n", 0x00FFFFFF);   
    return process->pid;
}

pid_t getCurrentPid() {
    PCB* current = getCurrentProcess(processes);
    return current ? current->pid : -1;
}

uint64_t schedule(uint64_t rsp){
    PCB* currentProcess = getCurrentProcess(processes);
    
    // Si no hay proceso actual, buscar uno
    if(currentProcess == NULL) {
        PCB* nextProcess = getNextProcess(processes);
        if (nextProcess == NULL) {
            return rsp;
        }
        nextProcess->state = RUNNING;
        currentPid = nextProcess->pid;
        quantum = calculateQuantum(nextProcess->priority);
        return nextProcess->rsp;
    }

    // Si el quantum no expiró, seguir con el mismo proceso
    if(quantum > 0 && currentProcess->state == RUNNING) {
        quantum--;
        return rsp; // MANTENER el RSP actual
    }
    
    // Quantum expiró, hacer context switch
    DEBUG_PRINT("Quantum expired, switching processes...\n", 0x00FFFFFF);
    
    // GUARDAR el contexto del proceso actual
    if(!first){
        currentProcess->rsp = rsp; // Guardar RSP actual
    } else {
        first = 0;
    }
    
    // Cambiar estado a READY (pero se queda en la cola)
    if(currentProcess->state == RUNNING) {
        currentProcess->state = READY;
    }

    // Obtener el siguiente proceso
    PCB* nextProcess = getNextProcess(processes);
    if (nextProcess == NULL) {
        DEBUG_PRINT("ERROR: No next process available!\n", 0x00FFFFFF);
        // Si no hay siguiente proceso, mantener el actual
        currentProcess->state = RUNNING;
        return rsp;
    }

    // Cambiar al siguiente proceso
    DEBUG_PRINT("Switching to process: ", 0x00FFFFFF);
    DEBUG_PRINT(nextProcess->name, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);
    
    nextProcess->state = RUNNING;
    currentPid = nextProcess->pid;
    quantum = calculateQuantum(nextProcess->priority);
    
    // RETORNAR el RSP del nuevo proceso
    return nextProcess->rsp;
}

uint64_t blockProcess (pid_t pid) {
    if(blockProcessQueue(processes, pid) != 0){
        return -1;
    }
    if (pid == getCurrentPid()) { // si el proceso bloqueado es el actual se renuncia al cpu con interrupción 
        yield(); 
    }
    return 0; 
}

void yield() {
    quantum = 0; // siguiente!!
    callTimerTick();
}

uint64_t unblockProcess(pid_t pid){
    PCB* process = getProcess(processes, pid);
    if (process == NULL || process->state != BLOCKED) {
        return -1;
    }
    process->state = READY;
    //readyProcesses++;
    return 0;
}

uint64_t kill(pid_t pid){
    if (pid == 0) { // Can't kill shell
        return -1;
    }
    
    PCB* process = getProcess(processes, pid);
    if (process == NULL) {
        DEBUG_PRINT("Process not found: ", 0x00FFFFFF);
        DEBUG_PRINT_INT(pid, 0x00FFFFFF);
        DEBUG_PRINT("\n", 0x00FFFFFF);
        return -1;
    }

    // Free process resources
    if (process->base != 0) {
        DEBUG_PRINT("Freeing memory for process: ", 0x00FFFFFF);
        DEBUG_PRINT(process->name, 0x00FFFFFF);
        DEBUG_PRINT("\n", 0x00FFFFFF);
        freeMemory((void*)(process->base - STACK_SIZE));
    }
    blockProcess(pid); // Set process state to BLOCKED before removing it
    process->state = EXITED; // Set process state to EXITED

    DEBUG_PRINT("Removing process: ", 0x00FFFFFF);
    DEBUG_PRINT(process->name, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);
    // If killing current process, force a reschedule
    if (pid == currentPid) {
        yield();
    }
    
    return 0;
}

int8_t changePrio(pid_t pid, int8_t newPrio){
    PCB* process = getProcess(processes, pid);
    if (process == NULL) {
        return -1;
    }
    if (newPrio < MIN_PRIORITY) {
        newPrio = MIN_PRIORITY;
    } else if (newPrio > MAX_PRIORITY) {
        newPrio = MAX_PRIORITY;
    }
    process->priority = newPrio;
    return newPrio;
}

PCB* getProcessInfo(uint64_t *cantProcesses){
    if ( processes == NULL) {
        *cantProcesses = 0;
        return NULL;
    }
    
    uint64_t count = countProcesses(processes);

    PCB* processInfo = allocMemory(sizeof(PCB) * count);
    if (processInfo == NULL) {
        *cantProcesses = 0;
        return NULL;
    }

    int found = 0;
    pid_t pid = 0; 
    
    for (uint64_t i = 0; found < count; i++) {
        PCB* process = getProcess(processes, pid);
        if (process != NULL) {

            if (copyProcess(&processInfo[found], process) == -1) {
                freeMemory(processInfo);
                *cantProcesses = 0;
                return NULL;
            }
            found++;
        } 
        pid++;
    }

    *cantProcesses = count;
    return processInfo;
}

int16_t copyProcess(PCB *dest, PCB *src) {
    dest->pid = src->pid;
    dest->parentPid = src->parentPid;
    dest->priority = src->priority;
    dest->state = src->state;
    dest->rsp = src->rsp;
    dest->base = src->base;
    dest->rip = src->rip;
	strncpy(dest->name, src->name, NAME_MAX_LENGTH);
	dest->name[NAME_MAX_LENGTH - 1] = '\0'; 
    return 0;
}

int32_t killCurrentProcess(int32_t retValue) {
	return kill(retValue);
}