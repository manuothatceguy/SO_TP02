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

// Implementaciones de las syscalls
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

#define CANT_REGS 18

uint64_t* syscall_getRegisters(uint64_t buff[]) {
    uint64_t *copy = getRegisters();
    memcpy((void*)buff,(const void *)copy,CANT_REGS*sizeof(void*)); // funcionara?
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

uint64_t syscall_time(int64_t timeZone, int64_t mod){
    int64_t time = getTimeParam(timeZone, mod);
    if(time > 0){
       return time; 
    } else {
        // error
    }
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
        return syscall_read(va_arg(ap,char*),va_arg(ap,uint64_t));
        break;
    case 2:
        return syscall_write(va_arg(ap,uint64_t),va_arg(ap,char*),va_arg(ap, uint64_t));
        break;
    case 3:
        return syscall_beep(va_arg(ap,uint64_t),va_arg(ap,uint64_t));
        break;
    case 4:
        return syscall_drawRectangle(va_arg(ap,Point2D*),va_arg(ap,Point2D*),va_arg(ap,uint32_t));
    case 5:
        return syscall_getRegisters(va_arg(ap, uint64_t*));
    case 6:
        return syscall_clearScreen();
    case 7:
        return syscall_fontSizeUp(); 
    case 8:
        return syscall_fontSizeDown();  
    case 9:
        return syscall_getHeight(); 
    case 10:
        return syscall_getWidth(); 
    default:
        break;
    }
    va_end(ap);
    return 0; // error
}