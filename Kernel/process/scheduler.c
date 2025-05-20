#include <scheduler.h>
#include <defs.h>
#include <memoryManager.h>
#include <interrupts.h>
#include <pcb.h>
#include <lib.h>
#include <textModule.h>

#define QUANTUM 10
#define MAX_PRIORITY 5 // agregar a limitaciones
#define MIN_PRIORITY 0
#define IDLE_PRIORITY -1

extern void wrapper();

uint64_t calculateQuantum(uint8_t priority) {
    if (priority == IDLE_PRIORITY) {
        return QUANTUM;  // Minimum quantum for idle
    }
    return QUANTUM * (MAX_PRIORITY - priority + 1);
}

static ProcessLinkedPtr processes = NULL;              
static pid_t currentPid = -1;   
static pid_t nextFreePid = 0; // 0 es el idle!
static uint64_t readyProcesses = 0; // cantidad de procesos listos
static uint64_t quantum = 0;    

void initScheduler(ProcessLinkedPtr list) {
    processes = list;
}

pid_t createProcess(char* name, fnptr function, uint64_t argc, char **arg, uint8_t priority) {
    _cli(); // Disable interrupts to make the function atomic
    
    if (name == NULL || function == NULL) {
        _sti(); // Re-enable interrupts before returning
        return -1;
    }

    if (argc > 0 && arg == NULL) {
        _sti(); // Re-enable interrupts before returning
        return -1;
    }

    PCB* process = allocMemory(sizeof(PCB));
    if (process == NULL) {
        _sti(); // Re-enable interrupts before returning
        return -1; 
    }

    if(priority < IDLE_PRIORITY){
        priority = IDLE_PRIORITY;
    } else if(priority > MAX_PRIORITY){
        priority = MAX_PRIORITY;
    }

    strncpy(process->name, name, NAME_MAX_LENGTH);
    process->pid = nextFreePid++;
    process->parentPid = currentPid;
    process->state = READY;
    readyProcesses++;
    
    process->priority = priority;
    process->base = (uint64_t)allocMemory(STACK_SIZE);

    if ((void *)process->base == NULL) {
        freeMemory(process); 
        _sti(); // Re-enable interrupts before returning
        return -1;
    }
    process->base += STACK_SIZE;

    process->rip = (uint64_t)wrapper;
    process->rsp = processStackFrame(process->base + STACK_SIZE, (uint64_t)function, argc, (uint64_t)arg);
     
    addProcess(processes, process);
    printStr("Creating process...\n", 0x00FFFFFF);   
    _sti(); // Re-enable interrupts before returning
    return process->pid;
}

pid_t getCurrentPid() {
    return currentPid;
} 

uint64_t schedule(uint64_t rsp){
    if(processes == NULL) {
        return rsp;
    }

    if(quantum > 0) {
        quantum--;
        return rsp;
    }
    printStr("Quantum expired, switching processes...\n", 0x00FFFFFF);
    // Quantum expired, need to switch processes
    PCB* currentProcess = getCurrentProcess(processes);
    if(currentProcess != NULL) {
        currentProcess->rsp = rsp;
        currentProcess->state = READY;
        readyProcesses++;
        printStr("Current process set to READY: ", 0x00FFFFFF);
        printStr(currentProcess->name, 0x00FFFFFF);
        printStr("\n", 0x00FFFFFF);
    }

    PCB* nextProcess = getNextProcess(processes);
    if(nextProcess == NULL) {
        printStr("No next process found, keeping current\n", 0x00FFFFFF);
        // No ready processes, keep current process
        if(currentProcess != NULL) {
            currentProcess->state = RUNNING;
            readyProcesses--;
            currentPid = currentProcess->pid;
        }
        quantum = calculateQuantum(currentProcess != NULL ? currentProcess->priority : MIN_PRIORITY);
        return rsp;
    }

    // Switch to next process
    printStr("Switching to process: ", 0x00FFFFFF);
    printStr(nextProcess->name, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    nextProcess->state = RUNNING;
    readyProcesses--;
    currentPid = nextProcess->pid;
    quantum = calculateQuantum(nextProcess->priority);
    return nextProcess->rsp;
}

uint64_t blockProcess (pid_t pid) {
    PCB* process = getProcess(processes, pid);
    if (process == NULL || pid == 0) { // no se puede bloquear el idle
        return -1;
    }
    process->state = BLOCKED;
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
    readyProcesses++;
    return 0;
}

uint64_t kill(pid_t pid){
    if (pid == 0) { // Can't kill idle process
        return -1;
    }
    
    PCB* process = getProcess(processes, pid);
    if (process == NULL) {
        return -1;
    }

    // Free process resources
    if (process->base != 0) {
        freeMemory((void*)(process->base - STACK_SIZE));
    }
    
    // Remove process from list
    removeProcess(processes, pid);
    
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