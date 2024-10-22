#include <time.h>
#include <stdint.h>
#include <clock.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <textModule.h>
#include <lib.h>

// Prototipos de las funciones de syscall
uint64_t syscall_write(uint64_t, char*, uint64_t);
void* syscall_time(int64_t);
uint64_t syscall_read(char *, uint64_t);
uint64_t syscall_beep(uint64_t, uint64_t);
uint64_t* syscall_getRegisters();

typedef struct Point2D{
    uint64_t x,y;
} Point2D;

uint64_t syscall_drawRectangle(Point2D*, Point2D *, uint32_t );

void* syscallDispatcher(uint64_t syscall_number, void* param1, void* param2, void* param3) {
    void* returnValue = 0;

    // Despachar la syscall según el número en rax
    switch (syscall_number) {
        case 1:
            returnValue = (void*)syscall_read((char*)param1, (uint64_t)param2);
        case 2:
            returnValue = (void*)syscall_write((uint64_t)param1,(char*) param2, (uint64_t)param3);
            break;
        case 3:
            returnValue = (void*)syscall_time((int64_t)param1);
            break;
        case 4:
            returnValue = (void*)syscall_beep((uint64_t)param1, (uint64_t)param2);
        case 5:
            returnValue = (void*)syscall_drawRectangle((Point2D*)param1, (Point2D*)param2, (uint64_t)param3);
        case 6:
            returnValue = (void*)syscall_getRegisters();
        default:
            break;
    }

    return returnValue;  // Devolver el valor al manejador en ensamblador
}

uint64_t syscall_write(uint64_t fd, char * buff , uint64_t length) {
    if(length < 0) return 1;
    if(fd > 2 || fd < 0) return 2;
    uint64_t color;
    switch (fd)
    {
    case 1: // STDOUT NORMAL
        color = 0x00FFFFFF;
        break;
    case 2: // STDOUT ERROR
        color = 0x00FF0000;
    default:
        return 3;
    }
    for(int i = 0; i < length; i++){
        putChar(buff[i],1,color);
    }
    return 0;
}

void* syscall_time(int64_t param1) {
    static time t = {0};  // Ensure the time structure is static to return its address
    t = getTime(param1);
    return &t;
}

uint64_t syscall_read(char * buff, uint64_t length){
    if(length <= 0) return -1;
    for(int i = 0; i < length; i++){
        buff[i] = getChar();
    }
    return 0;
}

uint64_t syscall_beep(uint64_t freq, uint64_t secs){
    return 0; // TODO implementar
}

uint64_t syscall_drawRectangle(Point2D* upLeft, Point2D *bottomRight, uint32_t color){
    return drawRectangle(upLeft->x,upLeft->y,bottomRight->y-upLeft->y,bottomRight->x-upLeft->x,color);
}

uint64_t* syscall_getRegisters(){
    return getRegisters();
}