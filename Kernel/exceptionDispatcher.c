#include <textModule.h>
#include <keyboardDriver.h>
#include <stdint.h>
#include <interrupts.h>
#include <lib.h>

#define RED 0x00FF0000
#define CANT_REGS 18

extern void init();

void printHex(uint64_t value, uint32_t color) {
    char hexStr[17];
    hexStr[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        uint8_t nibble = value & 0xF;
        if (nibble < 10) {
            hexStr[i] = '0' + nibble;
        } else {
            hexStr[i] = 'A' + (nibble - 10);
        }
        value >>= 4;
    }
    printStr(hexStr, color);
}

void exception(char * name, uint64_t rip) {
    toggleCursor(0);
    clearText(0);
    printStr(name,RED);
    printStr(" exception",RED);
    printStr("\n",RED);

    printStr("Estado de los registros: \n",RED);
    char * registersNames[CANT_REGS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: "};
    uint64_t registersRead[CANT_REGS];
    getRegisters(registersRead);
    for(int i = 0; i < CANT_REGS - 1; i++){
        printStr(registersNames[i],RED);
        printStr("0x",RED);
        printHex(registersRead[i],RED);
        printStr("\n",RED);
    }
    printStr("RIP: 0x",RED);
    printHex(rip,RED);
    printStr("\n",RED);
    printStr("Presiona cualquier tecla para volver.\n",RED);
    toggleCursor(0);
    while(getChar() == 0){ _hlt(); }
    clearScreen(0);
    init(); // vuelve al main
}

void exceptionDispatcher(int ex, uint64_t rip) {
    switch (ex) {
        case 0:
            exception("Zero division", rip);
            break;
        case 6:
            exception("Invalid opcode", rip);
            break;
        default:
            break;
    }
}