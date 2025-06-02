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

uint64_t calculateQuantum(uint8_t priority) {
    if (priority == IDLE_PRIORITY) {
        return QUANTUM;  // Minimum quantum for idle
    }
    return QUANTUM * (MAX_PRIORITY - priority + 1);
}

static ProcessManagerADT processes = NULL;              
static pid_t currentPid = -1;   
static pid_t nextFreePid = 0; // 0 es el idle!
static uint64_t quantum = 0;    

// Declaración de función interna
static PCB* _createProcessPCB(char* name, fnptr function, uint64_t argc, char **arg, uint8_t priority);

void initScheduler(fnptr idle) {
    ProcessManagerADT list = createProcessManager();
    PCB* idleProcess = _createProcessPCB("idle", idle, 0, NULL, IDLE_PRIORITY);
    setIdleProcess(list, idleProcess);
    processes = list;
}

pid_t createProcess(char* name, fnptr function, uint64_t argc, char **arg, uint8_t priority) {
    PCB* process = _createProcessPCB(name, function, argc, arg, priority);
    if (process == NULL) {
        return -1;
    }
    return process->pid;
}

static PCB* _createProcessPCB(char* name, fnptr function, uint64_t argc, char **arg, uint8_t priority) {
    if (name == NULL || function == NULL) {
        return NULL;
    }

    if (argc > 0 && arg == NULL) {
        return NULL;
    }

    PCB* process = allocMemory(sizeof(PCB));
    if (process == NULL) {
        return NULL; 
    }

    if(priority < IDLE_PRIORITY){
        priority = IDLE_PRIORITY;
    } else if(priority > MAX_PRIORITY){
        priority = MAX_PRIORITY;
    }

    strncpy(process->name, name, NAME_MAX_LENGTH);
    process->pid = nextFreePid++;
    process->waitingForPid = -1;
    process->retValue = 0;     

    if (currentPid == -1) {
        process->parentPid = -1; 
    } else {
        process->parentPid = currentPid;
    }

    process->state = READY;
    
    process->priority = priority;
    process->base = (uint64_t)allocMemory(STACK_SIZE);

    if ((void *)process->base == NULL) {
        freeMemory(process); 
        return NULL;
    }
    process->base += STACK_SIZE;
    process->rip = (uint64_t)function;
    process->rsp = processStackFrame(process->base, (uint64_t)function, argc, arg);
    if(priority != IDLE_PRIORITY) addProcess(processes, process);
    DEBUG_PRINT("Creating process...\n", 0x00FFFFFF);   
    return process;
}

pid_t getCurrentPid() {
    PCB* current = getCurrentProcess(processes);
    return current ? current->pid : -1;
}

uint64_t schedule(uint64_t rsp){
    static int first = 1;
    PCB* currentProcess = getCurrentProcess(processes);
    
    if(currentProcess == NULL) {
        PCB* nextProcess = getNextProcess(processes);
        if (nextProcess == NULL) {
            quantum = calculateQuantum(IDLE_PRIORITY);
            if(!first){
                return currentProcess->rsp = rsp;
            }
            first = 0;
            return ((PCB*)getIdleProcess(processes))->rsp;
        }
        nextProcess->state = RUNNING;
        currentPid = nextProcess->pid;
        quantum = calculateQuantum(nextProcess->priority);
        return nextProcess->rsp;
    }

    if(quantum > 0 && currentProcess->state == RUNNING) {
        quantum--;
        return rsp; // MANTENER el RSP actual
    }
    
    DEBUG_PRINT("Quantum expired, switching processes...\n", 0x00FFFFFF);

    currentProcess->rsp = rsp;
    if(currentProcess->state == RUNNING) {
        currentProcess->state = READY;
    }

    PCB* nextProcess = getNextProcess(processes); // Si no tengo READYs --> idle
    if (nextProcess == NULL) {
        first=0;
        return ((PCB*)getIdleProcess(processes))->rsp;
    }

    // Si tengo READYs --> lo cambio
    DEBUG_PRINT("Switching to process: ", 0x00FFFFFF);
    DEBUG_PRINT(nextProcess->name, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);
    
    nextProcess->state = RUNNING;
    currentPid = nextProcess->pid;
    quantum = calculateQuantum(nextProcess->priority);
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
    return unblockProcessQueue(processes, pid);
}

uint64_t kill(pid_t pid){
    if (pid == 0) { // Can't kill shell
        return -1;
    }
    //child->retValue es: (pid == getCurrentPid()) ? EXITED : KILLED; // si es el actual sale, sino es xq lo mataron
    PCB* process = killProcess(processes, pid, (pid == getCurrentPid()) ? EXITED : KILLED, ZOMBIE);
    if(process == NULL){
        return -1;
    }
    freeMemory((void*)process->base - STACK_SIZE); // libera el stack
    wakeupWaitingParent(process->parentPid, pid);
    if (pid == currentPid) {
        yield();
    }
    return 0;
}

void wakeupWaitingParent(pid_t parentPid, pid_t childPid) {
    if (parentPid == -1) return;
    
    PCB* parent = getProcess(processes, parentPid);
    if (parent != NULL && 
        parent->state == BLOCKED && 
        parent->waitingForPid == childPid) {
        unblockProcess(parentPid);
        DEBUG_PRINT("Parent process unblocked: ", 0x00FFFFFF);
        DEBUG_PRINT(parentPid, 0x00FFFFFF);
        DEBUG_PRINT("\n", 0x00FFFFFF);
        printStr("Parent process unblocked: ", 0x00FFFFFF);
        printInt(parentPid, 0x00FFFFFF);
        printStr("\n", 0x00FFFFFF);
    }
}

// Función auxiliar para reapear
int32_t reapChild(PCB* child, int32_t* status) {
    if (status != NULL) {
        *status = child->retValue;
    }
    
    int32_t childPid = child->pid;
    
    // Limpiar el proceso
    child->state = child->retValue; // EXITED O KILLED, MODIFICAR
    removeZombieProcess(processes, childPid);
    freeMemory(child);
    
    DEBUG_PRINT("Process reaped: ", 0x00FFFFFF);
    DEBUG_PRINT(childPid, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);
    
    return childPid;
}

int32_t waitpid(pid_t pid, int32_t* status) {
    pid_t currentProcPid = getCurrentPid();
    
    // Buscar el proceso hijo
    PCB* child = getProcess(processes, pid);
    if (child == NULL) {
        return -1; // Proceso no existe
    }
    
    // Verificar que sea hijo del proceso actual
    if (child->parentPid != currentProcPid) {
        return -1; // No es mi hijo
    }
    
    // Si el hijo ya está ZOMBIE, reapear inmediatamente
    if (child->state == ZOMBIE) {
        return reapChild(child, status);
    }
    
    // Si el hijo aún está corriendo, BLOQUEAR al padre
    if (child->state < ZOMBIE) {
        // Marcar que estamos esperando a este proceso específico
        PCB* parent = getProcess(processes, currentProcPid);
        parent->waitingForPid = pid;  // ← Nuevo campo en PCB
        
        // Bloquear al proceso padre
        printStr("Going to sleep", 0x00FFFFFF);
        blockProcess(currentProcPid);
        
        // Cuando el scheduler nos despierte, verificar de nuevo
        child = getProcess(processes, pid);
        if (child != NULL && child->state == ZOMBIE) {
            parent->waitingForPid = -1; // Limpiar
            return reapChild(child, status);
        }
    }
    
    return -1; // Error
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
    dest->waitingForPid = src->waitingForPid;
    dest->priority = src->priority;
    dest->state = src->state;
    dest->rsp = src->rsp;
    dest->base = src->base;
    dest->rip = src->rip;
	strncpy(dest->name, src->name, NAME_MAX_LENGTH);
	dest->name[NAME_MAX_LENGTH - 1] = '\0'; 
    dest->retValue = src->retValue;
    return 0;
}