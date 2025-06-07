#include <syscall.h>
#include <stdint.h>
#include <shared_structs.h>
#include <stdlib.h>
   
/*
 * @brief Realiza una interrupción 0x80 (SYSCALL)
 * @param code código de syscall
 * @param param... lo que corresponda para cada uno de los parámetros de la syscall. 0 si no se usa.
 * @return lo que devuelva la syscall
*/
extern int64_t syscall(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3);

enum syscall_number {
    NONE, 
    READ, 
    WRITE, 
    TIME, 
    BEEP, 
    DRAW_RECTANGLE, 
    GET_REGISTERS, 
    CLEAR_SCREEN, 
    SIZE_UP_FONT, 
    SIZE_DOWN_FONT, 
    GET_HEIGHT, 
    GET_WIDTH, 
    WAIT, 
    ALLOC_MEMORY, 
    FREE_MEMORY,
    CREATE_PROCESS,
    GETPID,
    KILL,
    BLOCK,
    UNBLOCK,
    CHANGE_PRIO,
    GET_PROCESS_INFO,
    MEM_INFO,
    EXIT,
    SEM_OPEN,
    SEM_WAIT,
    SEM_POST,
    SEM_CLOSE,
    YIELD,
    WAITPID,
    OPEN_PIPE,
    CLOSE_PIPE,
    CLEAR_PIPE
};

uint64_t syscall_read(uint64_t fd, char * buff, uint64_t len){
    return syscall(READ, fd, (uint64_t)buff, len);
}

uint64_t syscall_write(uint64_t fd, char * buff, uint64_t len){
    return syscall(WRITE, fd, (uint64_t)buff, len);
}

uint64_t syscall_time(uint64_t mod){
    return syscall(TIME, mod, 0, 0);
}

uint64_t syscall_beep(uint64_t freq, uint64_t ticks){
    return syscall(BEEP, freq, ticks, 0);
}

uint64_t syscall_drawRectangle(Point2D * ul, Point2D * br, uint32_t color){
    return syscall(DRAW_RECTANGLE, (uint64_t)ul, (uint64_t)br, (uint64_t)color);
}

uint64_t syscall_getRegisters(uint64_t * registers){
    return syscall(GET_REGISTERS, (uint64_t)registers, 0, 0);
}

uint64_t syscall_clearScreen(){
    return syscall(CLEAR_SCREEN, 0, 0, 0);
}

uint64_t syscall_sizeUpFont(uint64_t increment){
    return syscall(SIZE_UP_FONT, increment, 0, 0);
}

uint64_t syscall_sizeDownFont(uint64_t decrement){
    return syscall(SIZE_DOWN_FONT, decrement, 0, 0);
}

uint64_t syscall_getHeight(){
    return syscall(GET_HEIGHT, 0, 0, 0);
}

uint64_t syscall_getWidth(){
    return syscall(GET_WIDTH, 0, 0, 0);
}

uint64_t syscall_wait(uint64_t ticks){
    return syscall(WAIT, ticks, 0, 0);
}

void *syscall_allocMemory(uint64_t size) {
    return (void *)syscall(ALLOC_MEMORY, size, 0, 0);
}

void syscall_freeMemory(void *address) {
    syscall(FREE_MEMORY, (uint64_t)address, 0, 0);
}

uint64_t syscall_create_process(char *name, fnptr function, uint64_t argc, char *argv[], uint8_t priority, char foreground, int stdin, int stdout) {
    ProcessCreationParams params = {
        .name = name,
        .function = function,
        .argc = argc,
        .arg = argv,
        .priority = priority,
        .foreground = foreground,
        .stdin = stdin,
        .stdout = stdout
    };
    return syscall(CREATE_PROCESS, (uint64_t)&params, 0, 0);
}

uint64_t syscall_getpid() {
    return syscall(GETPID, 0, 0, 0);
}

uint64_t syscall_kill(uint64_t pid) {
    return syscall(KILL, pid, 0, 0);
}

uint64_t syscall_block(uint64_t pid) {
    return syscall(BLOCK, pid, 0, 0);
}

uint64_t syscall_unblock(uint64_t pid) {
    return syscall(UNBLOCK, pid, 0, 0);
}

int8_t syscall_changePrio(uint64_t pid, int8_t newPrio) {
    return syscall(CHANGE_PRIO, pid, newPrio, 0);
}

PCB *syscall_getProcessInfo(uint64_t *cantProcesses) {
    return (PCB *)syscall(GET_PROCESS_INFO, (uint64_t)cantProcesses, 0, 0);
}

int64_t syscall_memInfo(memInfo *info) {
    return syscall(MEM_INFO, (uint64_t)info, 0, 0);
}

void syscall_exit() {
    syscall(EXIT, 0, 0, 0);
}

int syscall_sem_open(int sem_id, uint64_t initialValue) {
    return syscall(SEM_OPEN, sem_id, initialValue, 0);
}

int syscall_sem_wait(int sem_id) {
    return syscall(SEM_WAIT, sem_id, 0, 0);
}

int syscall_sem_post(int sem_id) {
    return syscall(SEM_POST, sem_id, 0, 0);
}

int syscall_sem_close(int sem_id) {
    return syscall(SEM_CLOSE, sem_id, 0, 0);
}

void syscall_yield() {
    syscall(YIELD, 0, 0, 0);
}

int syscall_open_pipe() {
    return syscall(OPEN_PIPE, 0, 0, 0);
}

int syscall_close_pipe(int pipe_id) {
    return syscall(CLOSE_PIPE, pipe_id, 0, 0);
}
//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

int syscall_clear_pipe(int pipe_id) {
    return syscall(CLEAR_PIPE, pipe_id, 0, 0);
}

pid_t syscall_waitpid(pid_t pid, int32_t* status){
    return syscall(WAITPID, pid, (uint64_t)status, 0);
}   

int64_t my_getpid() {
  return 0;
}

int64_t my_create_process(char *name, uint64_t argc, char *argv[]) {
  return 0;
}

int64_t my_nice(uint64_t pid, uint64_t newPrio) {
  return 0;
}

int64_t my_kill(uint64_t pid) {
  return 0;
}

int64_t my_block(uint64_t pid) {
  return 0;
}

int64_t my_unblock(uint64_t pid) {
  return 0;
}

int64_t my_sem_open(char *sem_id, uint64_t initialValue) {
  return 0;
}

int64_t my_sem_wait(char *sem_id) {
  return 0;
}

int64_t my_sem_post(char *sem_id) {
  return 0;
}

int64_t my_sem_close(char *sem_id) {
  return 0;
}

int64_t my_yield() {
  return 0;
}

int64_t my_wait(int64_t pid) {
  return 0;
}
