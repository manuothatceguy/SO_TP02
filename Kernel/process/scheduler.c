#include <scheduler.h>
#include <defs.h>
#include <memoryManager.h>
#include <pcb.h>
#include <lib.h>

#define QUANTUM 10
#define MAX_PRIORITY 5 // agregar a limitaciones
#define MIN_PRIORITY 0

uint64_t calculateQuantum(uint8_t priority) {
    return QUANTUM * (MAX_PRIORITY - priority + 1);
}

static ProcessLinkedPtr processes = NULL;              
static pid_t currentPid = -1;   
static pid_t nextFreePid = 0; // 0 es el idle!
static uint64_t readyProcesses = 1; // cantidad de procesos listos
static uint64_t quantum = 0;    

void initScheduler(ProcessLinkedPtr list) {
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

    if(priority < MIN_PRIORITY){
        priority = MIN_PRIORITY;
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
        return -1;
    }
    process->base += STACK_SIZE;

    process->rip = (uint64_t)function;
    process->rsp = processStackFrame(process->base, process->rip, argc, arg); // HACER 
    addProcess(processes, process);
    return process->pid;
}

pid_t getCurrentPid() {
    return currentPid;
} 

uint64_t schedule(uint64_t rsp){
    if(processes == NULL || quantum-- > 0 ) {
        return rsp; 
    }
    
    if (quantum == 0) {
        PCB* currentProcess = getCurrentProcess(processes); 
        currentProcess->rsp = rsp; 
        currentProcess->state = READY; // cambiar el estado del proceso actual a listo
        readyProcesses++;
        currentProcess = getNextProcess(processes);
        quantum = calculateQuantum(currentProcess->priority); 
        currentProcess->state = RUNNING;
        readyProcesses--;
        return currentProcess->rsp;
    }
    return rsp;
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
    return 0; //HACER
}

uint64_t kill(pid_t pid){
    return 0; //hacer
}
