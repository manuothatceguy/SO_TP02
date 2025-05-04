#include <shellfunctions.h>
#include <syscall.h>
#include <stdlib.h>
#include <test_functions.h>

#define CANT_REGISTERS 19

extern void div_zero();
extern void invalid_opcode();

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

char * help =   " Lista de comandos disponibles:\n"        
                "    - exit: corta la ejecucion\n"
                "    - help: muestra este menu\n"
                "    - time: muestra la hora actual GMT-3\n"
                "    - registers: muestra el ultimo snapshot (tocar ESC)\n"
                "    - echo: imprime lo que le sigue a echo\n"
                "    - size_up: aumenta tamano de fuente\n"
                "    - size_down: decrementa tamano de fuente\n"
                "    - test_div_0: test zero division exception\n"
                "    - test_invalid_opcode: test invalid opcode exception\n"
                "    - clear: borra la pantalla y comienza arriba\n"
                "    - mem_status: imprime el estado de la memoria\n";

void showTime(){
    uint64_t time[] = {
        syscall_time(0), // secs
        syscall_time(1), // mins
        syscall_time(2), // hours
        syscall_time(3), // day
        syscall_time(4), // month
        syscall_time(5)  // year
    };
    char s[3] = {'0' + time[0] /10 % 10,'0' + time[0] % 10, 0};
    char m[3] = {'0' + time[1] /10 % 10, '0' + time[1] % 10, 0}; 
    char h[3] = {'0' + time[2] /10 % 10, '0' + time[2] % 10, 0};
    printf("Son las %s:%s:%s del %d de %s del %d\n", h, m, s, time[3], months[time[4]-1], time[5]);
}

void showRegisters(){    
    char * registersNames[CANT_REGISTERS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: ", "CS: "};
    uint64_t registersRead[CANT_REGISTERS];
    syscall_getRegisters(registersRead); 
    uint64_t aux = registersRead[7]; // asumiendo RSP [7] distinto de 0
    if(!aux){
        printf("No hay un guardado de registros. Presione ESC para hacer un backup\n");
        return;
    }
    for(int i = 0; i < CANT_REGISTERS ; i++){
        printf("Valor del registro %s %x \n", registersNames[i] , registersRead[i]);
    }
}

void handle_help(char * arg){
    printf("\n");
    printf(help); 
}

void handle_time(char * arg){
    showTime();
}

void handle_registers(char * arg){
    showRegisters();
}

void handle_echo(char * arguments){
    printf("%s\n", arguments);
}

void handle_size_up(char * arg){
    syscall_sizeUpFont(1);
    syscall_clearScreen();
}

void handle_size_down(char * arg){
    syscall_sizeDownFont(1);
    syscall_clearScreen();
}

void handle_test_div_0(char * arg){
    div_zero(); 
}

void handle_test_invalid_opcode(char * arg){
    invalid_opcode(); 
}

void handle_clear(char * arg){
    syscall_clearScreen();
}
//TODO : adapatar los llamados a los test dependiendo el caso
//       una vez que tengamos las funciones de creado de procesos
//      TODOS deben ser ejecutados como procesos(creacion de procesos) y no built-in de la shell
//      excepto test_mm que tambien debe andar en una aplicacion fuera del kernel
void handle_test_mm(char * arg){
    char *argv[] = { "100", NULL }; // Ejemplo de argumento, ajusta según sea necesario
    test_mm(1, argv);
}

void handle_test_prio(char * arg){
    char *argv[] = { NULL }; // Ajusta según sea necesario
    test_prio();
}

void handle_test_processes(char * arg){
    char *argv[] = { "5", NULL }; // Ejemplo de argumento, ajusta según sea necesario
    test_processes(1, argv);
}

void handle_test_sync(char * arg){
    char *argv[] = { "10", "1", NULL }; // Ejemplo de argumento, ajusta según sea necesario
    test_sync(2, argv);
}

void handle_mem_status(char * arg){
    //llama a syscall e imprime estado
}

