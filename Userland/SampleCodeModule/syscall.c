#include <syscall.h>
#include <stdint.h>
   
/*
 * @brief Realiza una interrupción 0x80 (SYSCALL)
 * @param code código de syscall
 * @param param... lo que corresponda para cada uno de los parámetros de la syscall. 0 si no se usa.
 * @return lo que devuelva la syscall
*/
extern int64_t syscall(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3);

enum syscall_number {NULL, READ, WRITE, TIME, BEEP, DRAW_RECTANGLE, GET_REGISTERS, CLEAR_SCREEN, SIZE_UP_FONT, SIZE_DOWN_FONT, GET_HEIGHT, GET_WIDTH, WAIT, TOGGLE_CURSOR};

uint64_t syscall_read(char * buff, uint64_t len){
    return syscall(READ, (uint64_t)buff, len, 0);
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

//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

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
