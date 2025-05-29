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
#include <stdio.h>
#include <semaphore.h>	


#define CANT_REGS 19
#define CANT_SYSCALLS 23

extern uint64_t regs[CANT_REGS];

typedef struct Point2D {
    uint64_t x, y;
} Point2D;

typedef uint64_t (*syscall_fn)(uint64_t rbx, uint64_t rcx, uint64_t rdx);


static uint64_t syscall_write(uint64_t fd, char *buff, uint64_t length) {
    if (length < 0) return 1;
    if (fd > 2 || fd < 0) return 2;
    uint64_t color = (fd == 1 ? 0x00FFFFFF : (fd == 2 ? 0x00FF0000 : 0));
    if(!color) return 0;
    for(int i = 0; i < length; i++)
        putChar(buff[i],color); 
    return length;  
}

static uint64_t syscall_beep(uint64_t freq, uint64_t ticks) {
    play_sound(freq);
    wait_ticks(ticks);
    nosound();
    return 0;
}

static uint64_t syscall_drawRectangle(Point2D* upLeft, Point2D *bottomRight, uint32_t color) {
    return drawRectangle(upLeft->x, upLeft->y, bottomRight->y - upLeft->y + 1, bottomRight->x - upLeft->x + 1, color);
}


static void syscall_getRegisters(uint64_t buff[]) {
    memcpy((void*)buff,(const void *)regs,CANT_REGS*(sizeof(void*)));
}

static uint64_t syscall_clearScreen(){
    clearText(0);
    return 0;
}

static uint64_t syscall_read( char* str,  uint64_t length){
    for(int i = 0; i < length && length > 0; i++){
        str[i] = getChar();
    }
    return length > 0 ? length : 0;
}

static uint64_t syscall_time(uint64_t mod){
    return getTimeParam(mod);
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

static uint64_t syscall_wait(uint64_t ticks){
    wait_ticks(ticks);
    return ticks;
}

pid_t syscall_create_process(ProcessCreationParams* params) {
    printStr("Process Creation Parameters KERNEL:\n", 0x00FFFFFF);
    printStr(params->name, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    printStr("Function: ", 0x00FFFFFF);
    printStr((char*)params->function, 0x00FFFFFF); 
    printStr("\n", 0x00FFFFFF);
    printStr("Argument Count: ", 0x00FFFFFF);
    printInt(params->argc, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    printStr("Arguments:\n", 0x00FFFFFF);
    for (uint64_t i = 0; i < params->argc; i++) {
        printStr("Arg: ", 0x00FFFFFF);
        printStr(params->arg[i], 0x00FFFFFF);
        printStr("\n", 0x00FFFFFF);
    }
    printStr("Priority: ", 0x00FFFFFF);
    printInt(params->priority, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);
    return createProcess(params->name, (fnptr)params->function, params->argc, params->arg, params->priority);
}

static uint64_t syscall_exit(){ // mata al proceso que llama a la syscall
    kill(getCurrentPid());
    return 0; // no deberia ejecutarse
}

pid_t syscall_getpid(){
    return getCurrentPid();
}

static uint64_t syscall_kill(uint64_t pid){
    return kill(pid);
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
    printStr("[KERNEL] temp.total:", 0x00ADD8E6);
    printInt(temp.total, 0x00ADD8E6);
    printStr(" temp.used:", 0x00ADD8E6);
    printInt(temp.used, 0x00ADD8E6);
    printStr(" temp.free:", 0x00ADD8E6);
    printInt(temp.free, 0x00ADD8E6);
    printStr("\n", 0x00ADD8E6);
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

uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3){
    if(syscall_number > CANT_SYSCALLS) return 0;
    syscall_fn syscalls[] = {0,
        (syscall_fn)syscall_read, 
        (syscall_fn)syscall_write, 
        (syscall_fn)syscall_time, 
        (syscall_fn)syscall_beep, 
        (syscall_fn)syscall_drawRectangle, 
        (syscall_fn)syscall_getRegisters, 
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
        (syscall_fn)syscall_yield
    };
    return syscalls[syscall_number](arg1, arg2, arg3);
}