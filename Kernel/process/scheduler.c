// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include <pipes.h>

#define SHELL_PID 1
#define TTY 0

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
static pid_t nextFreePid = 0; // PID 0 será asignado al proceso idle, PID 1 será la shell
static uint64_t quantum = 0;    

static PCB* _createProcessPCB(char* name, fnptr function, uint64_t argc, char **arg, int8_t priority, char foreground, int stdin, int stdout);
static void wakeupWaitingParent(pid_t parentPid, pid_t childPid) ;
int getCurrentProcessStdin();
int getCurrentProcessStdout();


void initScheduler(fnptr idle) {
    // Initialize pipe manager first
    initPipeManager();
    
    ProcessManagerADT list = createProcessManager();
    PCB* idleProcess = _createProcessPCB("idle", idle, 0, NULL, IDLE_PRIORITY, 0, -1, -1);
    setIdleProcess(list, idleProcess);
    processes = list;
}

pid_t createProcess(char* name, fnptr function, uint64_t argc, char **arg, int8_t priority, char foreground, int stdin, int stdout) {
    PCB* process = _createProcessPCB(name, function, argc, arg, priority, foreground, stdin, stdout);
    if (process == NULL) {
        return -1;
    }
    return process->pid;
}

static PCB* _createProcessPCB(char* name, fnptr function, uint64_t argc, char **arg, int8_t priority, char foreground, int stdin, int stdout) {
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
    process->foreground = foreground? 1 : 0; //programacion defensiva
    process->fds.stdin = -1;   // Initialize file descriptors to -1
    process->fds.stdout = -1;

    pid_t parent_pid = getCurrentPid();

    if (parent_pid == -1) {
        process->parentPid = -1; 
    } else {
        process->parentPid = parent_pid;
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

    if (process->pid > 1) { // proceso usuario
        if (!foreground && stdin == TTY) {
            process->fds.stdout = stdout;
            process->state = BLOCKED;
            addToBlockedQueue(processes, process);
            return process;
        }
        process->fds.stdin = (stdin == TTY) ? getCurrentProcessStdin() : stdin;
        process->fds.stdout = (stdout == 1) ? getCurrentProcessStdout() : stdout;

    } else if (process->pid == SHELL_PID) {
        process->fds.stdin = createPipe();
        if (process->fds.stdin < 0) {
            freeMemory((void*)process->base - STACK_SIZE);
            freeMemory(process);
            return NULL;
        }
        process->fds.stdout = 1;
    }
        

    if(priority != IDLE_PRIORITY) addProcess(processes, process);
    DEBUG_PRINT("Creating process...\n", 0x00FFFFFF);   
    return process;
}

pid_t getForegroundPid() {
    PCB* current = getForegroundProcess(processes);
    return current ? current->pid : -1;
}

pid_t getCurrentPid() {
    PCB* current = getCurrentProcess(processes);
    return current ? current->pid : -1;
}

uint64_t schedule(uint64_t rsp){
    static int first = 1;
    PCB* currentProcess = getCurrentProcess(processes);
    
    

    if(quantum > 0 && currentProcess->state == RUNNING) {
        quantum--;
        return rsp;
    }
    
    DEBUG_PRINT("Quantum expired, switching processes...\n", 0x00FFFFFF);

    if(!first) currentProcess->rsp = rsp; else first = 0;
    if(currentProcess->state == RUNNING) currentProcess->state = READY;

    PCB* nextProcess = getNextProcess(processes);

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

uint64_t blockProcessBySem(pid_t pid) {
    if (blockProcessQueueBySem(processes, pid) != 0) {
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
    return unblockProcessQueue(processes, pid, READY);
}

uint64_t unblockProcessBySem(pid_t pid) {
    return unblockProcessQueueBySem(processes, pid);
}

uint64_t kill(pid_t pid, uint64_t retValue) {
    if (pid <= SHELL_PID) { // Can't kill shell or idle
        return -1;
    }
    //child->retValue es: (pid == getCurrentPid()) ? EXITED : KILLED; // si es el actual sale, sino es xq lo mataron
    PCB* process = killProcess(processes, pid, retValue, ZOMBIE);
    if(process == NULL){
        return -1;
    }
    
    freeMemory((void*)process->base - STACK_SIZE); // libera el stack
    wakeupWaitingParent(process->parentPid, pid);
    if (pid == currentPid) {
        quantum = 0;
        callTimerTick();
    }
    return 0;
}

// Función auxiliar para reapear
static int32_t reapChild(PCB* child, int32_t* retValue) {
    if (retValue != NULL) {
        *retValue = child->retValue;
    }
    
    int32_t childPid = child->pid;
    
    // Limpiar el proceso
    child->state = child->retValue; // EXITED O KILLED, MODIFICAR
    removeZombieProcess(processes, childPid);
    freeMemory(child);
    
    DEBUG_PRINT("Process reaped: ", 0x00FFFFFF);
    DEBUG_PRINT_INT(childPid, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);
    
    return childPid;
}

static void wakeupWaitingParent(pid_t parentPid, pid_t childPid) {
    if (parentPid == -1){
        return;
    } 
    if(parentPid == getIdleProcess(processes)->pid){
        reapChild(getProcess(processes, childPid), NULL);
    }
    
    PCB* parent = getProcess(processes, parentPid);
    if (parent == NULL) {
        return;
    }

    DEBUG_PRINT("Waking parent PID ", 0x00FFFFFF);
    DEBUG_PRINT_INT(parentPid, 0x00FFFFFF);
    DEBUG_PRINT(" from child PID ", 0x00FFFFFF);
    DEBUG_PRINT_INT(childPid, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);

    if (parent->state == BLOCKED && parent->waitingForPid == childPid) {
        unblockProcess(parentPid);
    }
}

int32_t waitpid(pid_t pid, int32_t* retValue) {
    pid_t currentProcPid = getCurrentPid();
    
    DEBUG_PRINT("waitpid: Current PID ", 0x00FFFFFF);
    DEBUG_PRINT_INT(currentProcPid, 0x00FFFFFF);
    DEBUG_PRINT(" waiting for PID ", 0x00FFFFFF);
    DEBUG_PRINT_INT(pid, 0x00FFFFFF);
    DEBUG_PRINT("\n", 0x00FFFFFF);
    
    // Buscar el proceso
    PCB* target = getProcess(processes, pid);
    if (target == NULL) {
        return -1; // Proceso no existe
    }
    
    // Verificar que sea hijo del proceso actual o que estemos esperando por él como foreground
    PCB* current = getProcess(processes, currentProcPid);
    if (current == NULL) {
        return -1;
    }

    if (target->parentPid != currentProcPid && current->waitingForPid != pid) {
        return -1;
    }
    
    // Si el proceso ya está ZOMBIE, reapear inmediatamente
    if (target->state == ZOMBIE) {
        current->waitingForPid = -1;
        return reapChild(target, retValue);
    }
    
    // Si el proceso aún está corriendo, BLOQUEAR al proceso actual
    if (target->state < ZOMBIE) {
        DEBUG_PRINT("Blocking process ", 0x00FFFFFF);
        DEBUG_PRINT_INT(currentProcPid, 0x00FFFFFF);
        DEBUG_PRINT("\n", 0x00FFFFFF);
        
        // Primero marcar que estamos esperando a este proceso específico
        current->waitingForPid = pid;
        current->state = BLOCKED;
        
        // Luego bloquear al proceso actual
        if (blockProcess(currentProcPid) != 0) {
            current->waitingForPid = -1;
            current->state = READY;
            return -1;
        }
        // Cuando el scheduler nos despierte, verificar de nuevo el estado
        target = getProcess(processes, pid);
        if (target == NULL) {
            return -1;
        }
        
        if (target->state == ZOMBIE) {
            current->waitingForPid = -1;
            return reapChild(target, retValue);
        }
    }
    
    return -1;
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
    
    for (uint64_t i = 0; i < count; i++) {
        PCB* process = getProcess(processes, i);
        if (process != NULL) {

            if (copyProcess(&processInfo[i], process) == -1) {
                freeMemory(processInfo);
                *cantProcesses = 0;
                return NULL;
            }
        } 
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
    dest->foreground = src->foreground;
    dest->fds.stdin = src->fds.stdin;
    dest->fds.stdout = src->fds.stdout;
    return 0;
}

// Add new functions to access current process information
int getCurrentProcessStdin() {
    PCB* current = getCurrentProcess(processes);
    return current ? current->fds.stdin : -1;
}

int getCurrentProcessStdout() {
    PCB* current = getCurrentProcess(processes);
    return current ? current->fds.stdout : -1;
}
