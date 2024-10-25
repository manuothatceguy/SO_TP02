    #include <time.h>
#include <stdint.h>
#include <clock.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <textModule.h>
#include <lib.h>
#include <soundDriver.h>

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
        putChar(buff[i],2,color); // ajustar ese 2 correctamente
    return 0;  
}

uint64_t syscall_beep(uint64_t freq, int64_t secs) {
    play_sound(freq);
    wait_seconds(secs);
    nosound();
    return 0;
}

uint64_t syscall_drawRectangle(Point2D* upLeft, Point2D *bottomRight, uint32_t color) {
    return drawRectangle(upLeft->x, upLeft->y, bottomRight->y - upLeft->y, bottomRight->x - upLeft->x, color);
}

uint64_t* syscall_getRegisters() {
    return getRegisters();
}

uint64_t syscall_clearScreen(){
    clearScreen(0);
    return 0;
}

uint64_t syscall_read( char* str,  uint64_t length){
    for(int i = 0; i < length && length > 0; i++){
        str[i] = getChar();
    }
    return length > 0 ? length : 0;
}

time syscall_time(int64_t timeZone){ // TODO pensar como que le paso un struct time y "me lo llena"
    return getTime(timeZone);
}

typedef void* (*syscall_fn)(void*, void*, void*);



// Declaraciones de las funciones en formato genérico para el array
void* syscall_read_wrapper(void* param1, void* param2, void* param3) {
    return (void*)syscall_read((char*)param1, (uint64_t)param2);
}

void* syscall_write_wrapper(void* param1, void* param2, void* param3) {
    return (void*)syscall_write((uint64_t)param1, (char*)param2, (uint64_t)param3);
}

void* syscall_time_wrapper(void* param1, void* param2, void* param3) {
    return (void*)syscall_time((int64_t)param1);
}

void* syscall_beep_wrapper(void* param1, void* param2, void* param3) {
    return (void*)syscall_beep((uint64_t)param1, (uint64_t)param2);
}

void* syscall_drawRectangle_wrapper(void* param1, void* param2, void* param3) {
    return (void*)syscall_drawRectangle((Point2D*)param1, (Point2D*)param2, (uint32_t)param3);
    // el casteo de param3 es inválido --> revisar
    /*
    syscallDispatcher.c: In function 'syscall_drawRectangle_wrapper':
    syscallDispatcher.c:86:77: warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]
    86 |     return (void*)syscall_drawRectangle((Point2D*)param1, (Point2D*)param2, (uint32_t)param3);
    */
}

void* syscall_getRegisters_wrapper(void* param1, void* param2, void* param3) {
    return (void*)syscall_getRegisters();
}

void* syscall_clearScreen_wrapper(void *param1, void* param2, void* param3){
    return (void*)syscall_clearScreen();
}

// Array de punteros a función
syscall_fn syscalls[] = {
    0,                           // syscall 0: no utilizada
    syscall_read_wrapper,           // syscall 1: read
    syscall_write_wrapper,          // syscall 2: write
    syscall_time_wrapper,           // syscall 3: time
    syscall_beep_wrapper,           // syscall 4: beep
    syscall_drawRectangle_wrapper,  // syscall 5: draw rectangle
    syscall_getRegisters_wrapper,   // syscall 6: get registers
    syscall_clearScreen_wrapper     // syscall 7: clear screen

};

// Prototipos de las funciones de syscall
void* syscallDispatcher(uint64_t syscall_number, void* param1, void* param2, void* param3) {
    if (syscall_number > 0 && syscall_number < sizeof(syscalls) / sizeof(syscall_fn)) {
        // Llama a la función correspondiente a la syscall
        return syscalls[syscall_number](param1, param2, param3);
    }
    return 0;  // Si el número de syscall no es válido
}