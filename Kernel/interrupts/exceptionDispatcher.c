// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <textModule.h>
#include <keyboardDriver.h>
#include <stdint.h>
#include <interrupts.h>
#include <lib.h>

#define RED 0x00FF0000

extern void init();

void exception(char * name) {
    clearText(0);
    printStr(name,RED);
    printStr(" exception",RED);
    printStr("\n",RED);
    printStr("Presiona cualquier tecla para volver.\n",RED);
    while(getChar() == 0){ _hlt(); }
    clearText(0);
    init(); // vuelve al main
}

void exceptionDispatcher(int ex) {
    switch (ex) {
        case 0:
            exception("Zero division");
            break;
        case 6:
            exception("Invalid opcode");
            break;
        default:
            break;
    }
}