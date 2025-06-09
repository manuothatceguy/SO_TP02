// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <time.h>
#include <stdint.h>
#include <clock.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <textModule.h>
#include <lib.h>
#include <soundDriver.h>
#include <stdarg.h>
#include <memoryManager.h>
#include <defs.h>
#include <scheduler.h>
#include <shared_structs.h>
#include <debug.h>
#include <semaphore.h>	
#include <pipes.h>
#include <interrupts.h>

#define CANT_SYSCALLS 29

typedef uint64_t (*syscall_fn)(uint64_t rbx, uint64_t rcx, uint64_t rdx);


static uint64_t syscall_write(uint64_t fd, char *buff, uint64_t length) {
    if( fd == 1 ){
        fd = getCurrentProcessStdout();
    } else if( fd == 0 ){
        fd = getCurrentProcessStdin();
    }
    uint64_t color = fd == 1 ? 0x00FFFFFF : 0x00FF0000; // blanco o rojo
    switch(fd){
      case 0: // stdin
        return writePipe(0, buff, length);
        break;
      case 1: // stdout
      case 2: // stderr
        for(int i = 0; i < length && buff[i] != -1; i++)
            putChar(buff[i], color); 
        return length; 
        break;
      default: // pipe
        int64_t i;
        if(fd >= 3 + MAX_PIPES || (i = writePipe(fd, buff, length)) < 0) {
            return -1; // error
        }
        return i; 
        break;
    }
    return -1;
}

static uint64_t syscall_clearScreen(){
    clearText(0);
    return 0;
}

static uint64_t syscall_read(uint64_t fd, char* str,  uint64_t length){
    if(fd >= 3 + MAX_PIPES) {
        return -1; // error
    }
    if(fd == 0){ // stdin
        fd = getCurrentProcessStdin();
        if( fd == -1) {
            return -1; // error
        }
    }
    return readPipe(fd, str, length);
}

static uint64_t syscall_fontSizeUp(uint64_t increase){
    return fontSizeUp(increase);
}

static uint64_t syscall_fontSizeDown(uint64_t decrease){
    return fontSizeDown(decrease);
}

static uint64_t syscall_getWidth(){
    return getWidth();
}

static uint64_t syscall_getHeight(){
    return getHeight();
}

static uint64_t syscall_wait(uint64_t seconds){
    wait_seconds(seconds);
    return seconds;
}

static inline uint64_t getArgc(char** argv) {
    uint64_t argc = 0;
    if(argv == NULL || *argv == NULL) {
        return 0;
    }
    while (argv[argc] != NULL) {
        argc++;
    }
    return argc;
}

pid_t syscall_create_process(ProcessCreationParams* params) {
    return createProcess(params->name, (fnptr)params->function, getArgc(params->arg), params->arg, params->priority, params->foreground, params->stdin, params->stdout);
}

static uint64_t syscall_exit(uint64_t retValue){ // mata al proceso que llama a la syscall
    int eof = -1;
    syscall_write(1,&eof,1);
    kill(getCurrentPid(), retValue);
    return 0; // no deberia ejecutarse
}

pid_t syscall_getpid(){
    return getCurrentPid();
}

#define KILL 9

static uint64_t syscall_kill(uint64_t pid){
    return kill(pid, KILL);
}

pid_t syscall_block(uint64_t pid){
    return blockProcess(pid);
}

static uint64_t syscall_unblock(uint64_t pid){
    return unblockProcess(pid);
}

static uint64_t syscall_allocMemory(uint64_t size) {
    return (uint64_t)allocMemory( size);
}

static uint64_t syscall_freeMemory(uint64_t address) {
    freeMemory((void*)address);
    return 0;
}
static int8_t syscall_changePrio(uint64_t pid, int8_t newPrio) {
    return changePrio(pid, newPrio);
}

static PCB* syscall_getProcessInfo(uint64_t *cantProcesses) {
    return getProcessInfo(cantProcesses);
}

static int64_t syscall_memInfo(memInfo *user_ptr){
    if (user_ptr == NULL){
        return -1;
    }
    memInfo temp;
    getMemoryInfo(&temp);
    user_ptr->total = temp.total;
    user_ptr->used  = temp.used;
    user_ptr->free  = temp.free;

    return 0;
}

int syscall_sem_open(int sem_id, uint64_t initialValue) {
    if (sem_id < 0 || sem_id >= NUM_SEMS) {
        return -1; 
    }
    return semInit(sem_id, initialValue);
}

int syscall_sem_wait(int sem_id) {
    if (sem_id < 0 || sem_id >= NUM_SEMS) {
        return -1; 
    }
    return semWait(sem_id);
}

int syscall_sem_post(int sem_id) {
    if (sem_id < 0 || sem_id >= NUM_SEMS) {
        return -1; 
    }
    return semPost(sem_id);
}

int syscall_sem_close(int sem_id) {
    if (sem_id < 0 || sem_id >= NUM_SEMS) {
        return -1; 
    }
    return semClose(sem_id);
}

void syscall_yield() {
    yield();
}

pid_t syscall_waitpid(pid_t pid, int32_t* retValue) {
    if(pid < 0) {
        return -1;
    }
    return waitpid(pid, retValue);
}

int syscall_open_pipe() {
    return createPipe();
}

int syscall_close_pipe(int pipe_id) {
    return closePipe(pipe_id);
}

int syscall_clear_pipe(int pipe_id) {
    return clearPipe(pipe_id);
}

uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3){
    if(syscall_number >= CANT_SYSCALLS) return 0;
    _cli();
    syscall_fn syscalls[] = {0,
        (syscall_fn)syscall_read, 
        (syscall_fn)syscall_write, 
        (syscall_fn)syscall_clearScreen, 
        (syscall_fn)syscall_fontSizeUp, 
        (syscall_fn)syscall_fontSizeDown, 
        (syscall_fn)syscall_getHeight, 
        (syscall_fn)syscall_getWidth, 
        (syscall_fn)syscall_wait,
        (syscall_fn)syscall_allocMemory,
        (syscall_fn)syscall_freeMemory,
        (syscall_fn)syscall_create_process,
        (syscall_fn)syscall_getpid,
        (syscall_fn)syscall_kill,
        (syscall_fn)syscall_block,
        (syscall_fn)syscall_unblock,
        (syscall_fn)syscall_changePrio,
        (syscall_fn)syscall_getProcessInfo,
        (syscall_fn)syscall_memInfo,
        (syscall_fn)syscall_exit,
        (syscall_fn)syscall_sem_open,
        (syscall_fn)syscall_sem_wait,
        (syscall_fn)syscall_sem_post,
        (syscall_fn)syscall_sem_close,
        (syscall_fn)syscall_yield,
        (syscall_fn)syscall_waitpid,
        (syscall_fn)syscall_open_pipe,
        (syscall_fn)syscall_close_pipe,
        (syscall_fn)syscall_clear_pipe
    };
    uint64_t ret = syscalls[syscall_number](arg1, arg2, arg3);
    _sti();
    return ret;
}