#include <scheduler.h>
#include <defs.h>
#include <memoryManager.h>
#include <interrupts.h>
#include <pcb.h>
#include <lib.h>
#include <textModule.h>

#define QUANTUM 10

static inline uint64_t calculateQuantum(uint8_t priority) {
    if (priority == IDLE_PRIORITY) {
        return QUANTUM;  // Minimum quantum for idle
    }
    return QUANTUM * (MAX_PRIORITY - priority + 1);
}

static ProcessListADT processManager = NULL;              
static uint64_t quantum = 0;    

void initScheduler() {
    processManager = createProcessLinkedList();
}

pid_t getCurrentPid() {
    if(processManager == NULL) {
        return -1;
    }
    return getRunningProcess(processManager)->pid;
} 

uint64_t schedule(uint64_t rsp){
    if(processManager == NULL) {
        return rsp;
    }

    if(quantum > 0) {
        quantum--;
        return rsp;
    }
    printStr("Quantum expired, switching processes...\n", 0x00FFFFFF);
    // Quantum expired, need to switch processes
    PCB* runningProcess = getRunningProcess(processManager);
    if(runningProcess != NULL) {
        runningProcess->rsp = rsp;
        printStr("Current process set to READY: ", 0x00FFFFFF);
        printStr(runningProcess->name, 0x00FFFFFF);
        printStr("\n", 0x00FFFFFF);
    }

    PCB* nextProcess = getNextProcess(processManager);

    // Switch to next process
    printStr("Switching to process: ", 0x00FFFFFF);
    printStr(nextProcess->name, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    quantum = calculateQuantum(nextProcess->priority);
    return nextProcess->rsp;
}

uint64_t blockProcess (pid_t pid) {
    return processManager ? readyToBlocked(processManager, pid) : -1;
}

void yield() {
    quantum = 0; // siguiente!!
    callTimerTick();
}

uint64_t unblockProcess(pid_t pid){
    return processManager ? blockedToReady(processManager, pid) : -1;
}

uint64_t kill(pid_t pid){
    return -1; // implementar........
}

int8_t changePrio(pid_t pid, int8_t newPrio){
    if(processManager == NULL) {
        return -1;
    }
    return changePriority(processManager, pid, newPrio);
}

pid_t createProcess(char* name, fnptr function, char **argv, uint8_t priority){
    if(processManager == NULL) {
        return -1;
    }
    return createReadyProcess(processManager, name, function, argv, priority);
}