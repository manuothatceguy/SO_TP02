#include <shellfunctions.h>
#include <syscall.h>
#include <stdlib.h>

#define CANT_REGISTERS 18

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
        syscall(3,0,0,0), // secs
        syscall(3,1,0,0), // mins
        syscall(3,2,0,0), // hour
        syscall(3,3,0,0), // day
        syscall(3,4,0,0), // month
        syscall(3,5,0,0)  // year
    };
    printf("Son las %d:%d:%d del %d de %s de %d\n", time[0], time[1], time[2], time[3], months[time[4]-1], time[5]);
}

void showRegisters(){    
    char * registersNames[CANT_REGISTERS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: "};
    uint64_t registersRead[CANT_REGISTERS];
    syscall(6, registersRead, 0, 0); 
    uint64_t aux = registersRead[7]; // asumiendo RSP (7) distinto de 0
    if(!aux){
        printf("No hay un guardado de registros. Presione ESC para hacer un backup\n");
        return;
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
    syscall(7, 0, 0, 0);
}


