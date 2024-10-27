#include <shellfunctions.h>
#include <syscall.h>

#define CANT_REGISTERS 18

#include <stdlib.h>

char *months[] = {
    "Enero",
    "Febrero",
    "Marzo",
    "Abril",
    "Mayo",
    "Junio",
    "Julio",
    "Agosto",
    "Septiembre",
    "Octubre",
    "Noviembre",
    "Diciembre"
};

void showTime(){
    uint64_t time[] = {
        syscall(3,2,-3,0),
        syscall(3,2,-3,1),
        syscall(3,2,-3,2),
        syscall(3,2,-3,3),
        syscall(3,2,-3,4),
        syscall(3,2,-3,5)
    };
    printf("Son las %d:%d:%d del %d de %s de %d\n", time[0], time[1], time[2], time[3], months[time[4]-1], time[5]);
}

void showRegisters(){    
    char * registersNames[CANT_REGISTERS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "R8: ", "R9: ", "R10: ", "R11: ", "R12: ",
                                            "R13: ", "R14: ", "R15: ", "RSP: ", "RIP: ", "RFLAGS: "};
    uint64_t registersRead[CANT_REGISTERS];
    syscall(6, 1, registersRead);
    for(int i = 0; i < CANT_REGISTERS ; i++){
        printf("Valor del registro %s %x \n", registersNames[i] , registersRead[i]);
    }
}

void divZero(){

}

void invalidOpCode(){

}

void clear(){
    syscall(7, 0);
}


