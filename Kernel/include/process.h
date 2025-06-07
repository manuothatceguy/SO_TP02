#ifndef PROCESS_H
#define PROCESS_H

//#include <pcb.h>
#include <shared_structs.h>

#define STACK_SIZE 4096 
#define IDLE_PRIORITY -1

typedef struct ProcessManagerCDT *ProcessManagerADT;

/**
 * @brief Creates a new process manager
 * @return ProcessManagerADT New process manager instance
 */
ProcessManagerADT createProcessManager();

/**
 * @brief Adds a new process to the manager
 * @param list Process manager
 * @param process Process to add
 * @param foreground Whether the process is foreground (1) or background (0)
 */
void addProcess(ProcessManagerADT list, PCB *process, char foreground);

/**
 * @brief Removes a process from the ready queue
 * @param list Process manager
 * @param pid Process ID to remove
 */
void removeProcess(ProcessManagerADT list, pid_t pid);

/**
 * @brief Removes a process from the zombie queue
 * @param list Process manager
 * @param pid Process ID to remove
 */
void removeZombieProcess(ProcessManagerADT list, pid_t pid);

/**
 * @brief Frees all resources used by the process manager
 * @param list Process manager to free
 */
void freeProcessLinkedList(ProcessManagerADT list);

/**
 * @brief Gets a process by its PID from any queue
 * @param list Process manager
 * @param pid Process ID to find
 * @return PCB* Found process or NULL if not found
 */
PCB* getProcess(ProcessManagerADT list, pid_t pid);

/**
 * @brief Gets the next process to run from the ready queue
 * @param list Process manager
 * @return PCB* Next process or NULL if none available
 */
PCB* getNextProcess(ProcessManagerADT list);

/**
 * @brief Gets the currently running process
 * @param list Process manager
 * @return PCB* Current process or NULL if none
 */
PCB* getCurrentProcess(ProcessManagerADT list);

/**
 * @brief Gets the idle process
 * @param list Process manager
 * @return PCB* Idle process
 */
PCB* getIdleProcess(ProcessManagerADT list);

/**
 * @brief Sets the idle process
 * @param list Process manager
 * @param idleProcess Process to set as idle
 */
void setIdleProcess(ProcessManagerADT list, PCB* idleProcess);

/**
 * @brief Sets the foreground process
 * @param list Process manager
 * @param foregroundProcess Process to set as foreground
 */
void setForegroundProcess(ProcessManagerADT list, PCB* foregroundProcess);

/**
 * @brief Blocks a process
 * @param list Process manager
 * @param pid Process ID to block
 * @return int 0 on success, -1 on failure
 */
int blockProcessQueue(ProcessManagerADT list, pid_t pid);

/**
 * @brief Unblocks a process
 * @param list Process manager
 * @param pid Process ID to unblock
 * @param state New state for the process
 * @return int 0 on success, -1 on failure
 */
int unblockProcessQueue(ProcessManagerADT list, pid_t pid, ProcessState state);

/**
 * @brief Counts ready processes
 * @param list Process manager
 * @return uint64_t Number of ready processes
 */
uint64_t countReadyProcesses(ProcessManagerADT list);

/**
 * @brief Counts blocked processes
 * @param list Process manager
 * @return uint64_t Number of blocked processes
 */
uint64_t countBlockedProcesses(ProcessManagerADT list);

/**
 * @brief Counts total processes
 * @param list Process manager
 * @return uint64_t Total number of processes
 */
uint64_t countProcesses(ProcessManagerADT list);

/**
 * @brief Kills a process
 * @param list Process manager
 * @param pid Process ID to kill
 * @param retValue Return value for the process
 * @param state Final state for the process
 * @return PCB* Killed process or NULL on failure
 */
PCB* killProcess(ProcessManagerADT list, pid_t pid, uint64_t retValue, ProcessState state);

/**
 * @brief Blocks a process by semaphore
 * @param list Process manager
 * @param pid Process ID to block
 * @return int 0 on success, -1 on failure
 */
int blockProcessQueueBySem(ProcessManagerADT list, pid_t pid);

/**
 * @brief Unblocks a process blocked by semaphore
 * @param list Process manager
 * @param pid Process ID to unblock
 * @return int 0 on success, -1 on failure
 */
int unblockProcessQueueBySem(ProcessManagerADT list, pid_t pid);

/**
 * @brief Gets the current foreground process
 * @param list Process manager
 * @return PCB* Foreground process or NULL if none
 */
PCB* getForegroundProcess(ProcessManagerADT list);

/**
 * @brief Checks if a process is the foreground process
 * @param list Process manager
 * @param pid Process ID to check
 * @return char 1 if foreground, 0 if not
 */
char isCurrentForegroundProcess(ProcessManagerADT list, pid_t pid);

/**
 * @brief Checks if a process is the foreground process
 * @param process Process to check
 * @return char 1 if foreground, 0 if not
 */
char isForegroundProcess(PCB* process);

/**
 * @brief Checks if a process is in any queue
 * @param list Process manager
 * @param pid Process ID to check
 * @return int 1 if in a queue, 0 if not
 */
int isInAnyQueue(ProcessManagerADT list, pid_t pid);

/**
 * @brief Adds a process to the ready queue
 * @param list Process manager
 * @param process Process to add
 */
void addToReadyQueue(ProcessManagerADT list, PCB* process);

// Assembly functions
uint64_t processStackFrame(uint64_t base, uint64_t rip, uint64_t argc, char **arg);
void wrapper(uint64_t function, char **argv);

#endif // PROCESS_H