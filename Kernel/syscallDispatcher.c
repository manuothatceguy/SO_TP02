#include <time.h>
#include <stdint.h>
#include <clock.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <textModule.h>
#include <lib.h>
#include <soundDriver.h>
#include <stdarg.h>


#define CANT_REGS 18
#define CANT_SYSCALLS 12

extern uint64_t regs[CANT_REGS];

typedef struct Point2D {
    uint64_t x, y;
} Point2D;
typedef uint64_t (*syscall_fn)(uint64_t rbx, uint64_t rcx, uint64_t rdx);


static uint64_t syscall_write(uint64_t fd, char *buff, uint64_t length) {
    if (length < 0) return 1;
    if (fd > 2 || fd < 0) return 2;
    uint64_t color;
    switch (fd) {
        case 1: // STDOUT NORMAL
            color = 0x00FFFFFF;
            break;
        case 2: // STDOUT ERROR
            color = 0x00FF0000;
            break;
        default:
            return 3;
    }
    for(int i = 0; i < length; i++)
        putChar(buff[i],color); 
    return 0;  
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
    memcpy((void*)buff,(const void *)regs,CANT_REGS*(sizeof(void*))); // funcionara?
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

// Prototipos de las funciones de syscall
uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3){
    if(syscall_number > CANT_SYSCALLS) return 0;
    syscall_fn syscalls[] = {0,(syscall_fn)syscall_read, (syscall_fn)syscall_write, (syscall_fn)syscall_time, (syscall_fn)syscall_beep, (syscall_fn)syscall_drawRectangle, (syscall_fn)syscall_getRegisters, (syscall_fn)syscall_clearScreen, (syscall_fn)syscall_fontSizeUp, (syscall_fn)syscall_fontSizeDown, (syscall_fn)syscall_getHeight, (syscall_fn)syscall_getWidth, (syscall_fn)syscall_wait};
    return syscalls[syscall_number](arg1, arg2, arg3);
}