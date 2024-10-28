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
        syscall(3,2,-3,0), // secs
        syscall(3,2,-3,1), // mins
        syscall(3,2,-3,2), // hour
        syscall(3,2,-3,3), // day
        syscall(3,2,-3,4), // month
        syscall(3,2,-3,5)  // year
    };
    printf("Son las %d:%d:%d del %d de %s de %d\n", time[0], time[1], time[2], time[3], months[time[4]-1], time[5]);
}

void showRegisters(){    
    char * registersNames[CANT_REGISTERS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: "};
    uint64_t registersRead[CANT_REGISTERS];
    uint64_t aux = syscall(6, 1, registersRead);
    if(!aux){
        printf("No hay un guardado de registros. Presione ESC para hacer un backup\n");
    }
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


