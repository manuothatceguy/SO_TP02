#include <time.h>
#include <stdint.h>
#include <clock.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <textModule.h>
#include <lib.h>
#include <soundDriver.h>
#include <stdarg.h>

typedef struct Point2D {
    uint64_t x, y;
} Point2D;

#define CANT_REGS 18
extern uint64_t regs[CANT_REGS];

uint64_t syscall_write(uint64_t fd, char *buff, uint64_t length) {
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
        putChar(buff[i],color); // ajustar ese 2 correctamente
    return 0;  
}

uint64_t syscall_beep(uint64_t freq, uint64_t secs) {
    play_sound(freq);
    wait_seconds(secs);
    nosound();
    return 0;
}

uint64_t syscall_drawRectangle(Point2D* upLeft, Point2D *bottomRight, uint32_t color) {
    return drawRectangle(upLeft->x, upLeft->y, bottomRight->y - upLeft->y + 1, bottomRight->x - upLeft->x + 1, color);
}


void syscall_getRegisters(uint64_t buff[]) {
    memcpy((void*)buff,(const void *)regs,CANT_REGS*(sizeof(void*))); // funcionara?
}

uint64_t syscall_clearScreen(){
    clearText(0);
    return 0;
}

uint64_t syscall_read( char* str,  uint64_t length){
    for(int i = 0; i < length && length > 0; i++){
        str[i] = getChar();
    }
    return length > 0 ? length : 0;
}

uint64_t syscall_time(uint64_t mod){
    return getTimeParam(mod);
}

uint64_t syscall_fontSizeUp(){
    return fontSizeUp();
}

uint64_t syscall_fontSizeDown(){
    return fontSizeDown();
}

uint64_t syscall_getWidth(){
    return getWidth();
}

uint64_t syscall_getHeight(){
    return getHeight();
}

// Prototipos de las funciones de syscall
uint64_t syscallDispatcher(uint64_t syscall_number, ...) {
    va_list ap;
    va_start(ap,syscall_number);
    switch (syscall_number)
    {
    case 1:
        char * param1_read = va_arg(ap,char*);
        uint64_t param2_read = va_arg(ap,uint64_t);
        va_end(ap);
        return syscall_read(param1_read,param2_read);
    case 2:
        uint64_t param1_write = va_arg(ap,uint64_t);
        char * param2_write = va_arg(ap,char*);
        uint64_t param3_write = va_arg(ap,uint64_t); 
        va_end(ap);
        return syscall_write(param1_write,param2_write,param3_write);
    case 3:
        uint64_t param_time = va_arg(ap,uint64_t);
        va_end(ap);
        return syscall_time(param_time);
    case 4:
        uint64_t param1_beep = va_arg(ap,uint64_t);
        uint64_t param2_beep = va_arg(ap,uint64_t);
        va_end(ap);
        return syscall_beep(param1_beep,param2_beep);
    case 5:
        Point2D * param1_rectangle = va_arg(ap,Point2D*);
        Point2D * param2_rectangle = va_arg(ap,Point2D*);
        uint32_t param3_rectangle = va_arg(ap,uint32_t);
        va_end(ap);
        return syscall_drawRectangle(param1_rectangle,param2_rectangle,param3_rectangle);
    case 6:
        uint64_t * param_registers = va_arg(ap,uint64_t*);
        va_end(ap);
        syscall_getRegisters(param_registers);
        return 0;
    case 7:
        va_end(ap);
        return syscall_clearScreen();
    case 8:
        va_end(ap);
        return syscall_fontSizeUp(); 
    case 9:
        va_end(ap);
        return syscall_fontSizeDown();  
    case 10:
        va_end(ap);
        return syscall_getHeight(); 
    case 11:
        va_end(ap);
        return syscall_getWidth(); 
    default:
        break;
    }
    return 0; // error
}