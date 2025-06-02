#include "shellfunctions.h"
#include <syscall.h>
#include <stdlib.h>
#include <test_functions.h>
#include <shared_structs.h>

#define CANT_REGISTERS 19

extern void div_zero();
extern void invalid_opcode();

//uint64_t syscall_create_process(char *name, fnptr function, uint64_t argc, char *argv[], uint8_t priority);

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
                "    - test_mm <max_memory>: test de gestion de memoria\n"
                "    - test_processes <max_processes>: test de procesos\n"
                "    - test_prio: test de prioridades\n"
                "    - test_sync: test de sincronizacion\n"
                "    - ps: muestra los procesos con su informacion\n"
                "    - memInfo: imprime estado de la memoria\n"
                "    - loop <time>: ejecuta un bucle por el tiempo especificado\n";
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

void handle_mem_info(char * arg) {
    free(arg);
    printf("Estado de memoria:\n");

    memInfo info;
    if (syscall_memInfo(&info) == -1) {
        printf("Error al obtener el estado de memoria\n");
        return;
    }
    
    printf("Memoria total: %d bytes\n", info.total);
    printf("Memoria usada: %d bytes\n", info.used);
    printf("Memoria libre: %d bytes\n", info.free);
    
    return;
}

void handle_test_mm(char * arg) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_mm <max_memory>\n");
        return;
    }
    printf("Iniciando test de gestion de memoria...\n");
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return;
    }
    argv[0] = arg; // Asignar el argumento
    argv[1] = NULL; // Terminar el array de argumentos
    printf("arg: %s\n", arg);
    printf("Direccion de test_mm: %p\n", (uint64_t*)test_mm);
    // Crear un nuevo proceso para ejecutar el test
    pid_t pid = syscall_create_process("test_mm", (fnptr)test_mm, 1, argv, 1);

    if (pid < 0) {
        printf("Error al crear el proceso de test\n");
    } else {
        printf("Proceso de test creado con PID: %d\n", pid);
    }
}

void handle_test_processes(char * arg) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_processes <max_processes>\n");
        return;
    }

    uint64_t max_processes = 0;
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] >= '0' && arg[i] <= '9') {
            max_processes = max_processes * 10 + (arg[i] - '0');
        } else {
            printf("Error: argumento invalido\n");
            return;
        }
    }
    
    if (max_processes <= 0) {
        printf("Error: max_processes debe ser mayor que 0\n");
        return;
    }
    
    printf("Iniciando test de procesos...\n");
    char *argv[] = { arg, NULL };

    pid_t pid = syscall_create_process("test_processes", (fnptr)test_processes, 1, argv, 1);
    
    if (pid < 0) {
        printf("Error al crear el proceso de test\n");
    } else {
        printf("Proceso de test creado con PID: %d\n", pid);
    }
}

void handle_test_prio(char * arg) {
    printf("Iniciando test de prioridades...\n");
    char *argv[] = { NULL };
    
    // Crear un nuevo proceso para ejecutar el test
    pid_t pid = syscall_create_process("test_prio", (fnptr)test_prio, 0, argv, 1);
    
    if (pid < 0) {
        printf("Error al crear el proceso de test\n");
    } else {
        printf("Proceso de test creado con PID: %d\n", pid);
    }
}

void handle_test_sync(char * arg) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_sync <iterations> <processes>\n");
        return;
    }
    
    // Encontrar el primer espacio
    int space_pos = -1;
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] == ' ') {
            space_pos = i;
            break;
        }
    }
    
    if (space_pos == -1) {
        printf("Error: se requieren dos argumentos\n");
        return;
    }
    
    // Crear copias de los argumentos
    char iterations[32] = {0};
    char processes[32] = {0};
    
    // Copiar primer argumento
    for (int i = 0; i < space_pos; i++) {
        iterations[i] = arg[i];
    }
    
    // Copiar segundo argumento
    int j = 0;
    for (int i = space_pos + 1; arg[i] != '\0'; i++) {
        processes[j++] = arg[i];
    }
    
    // Verificar que ambos sean números válidos
    for (int i = 0; iterations[i] != '\0'; i++) {
        if (iterations[i] < '0' || iterations[i] > '9') {
            printf("Error: iterations debe ser un número\n");
            return;
        }
    }
    
    for (int i = 0; processes[i] != '\0'; i++) {
        if (processes[i] < '0' || processes[i] > '9') {
            printf("Error: processes debe ser un número\n");
            return;
        }
    }
    
    printf("Iniciando test de sincronizacion...\n");
    char *argv[] = { iterations, processes, NULL };
    
    // Crear un nuevo proceso para ejecutar el test
    pid_t pid = syscall_create_process("test_sync", (fnptr)test_sync, 2, argv, 1);
    
    if (pid < 0) {
        printf("Error al crear el proceso de test\n");
    } else {
        printf("Proceso de test creado con PID: %d\n", pid);
    }
}

static void printProcessInfo(PCB processInfo) {
    const char* state;
    switch(processInfo.state) {
        case READY:   state = "READY";   break;
        case RUNNING: state = "RUNNING"; break;
        case BLOCKED: state = "BLOCKED"; break;
        case ZOMBIE:  state = "ZOMBIE";  break;
        case EXITED:  state = "EXITED";  break;
        case KILLED:  state = "KILLED";  break;
        case WAITING_SEM: state = "WAITING_SEM"; break;
        default:      state = "UNKNOWN"; break;
    }

    printf("\n");
    printf("Name        : %s\n", processInfo.name);
    printf("PID         : %d\n", (int) processInfo.pid);
    
    if (processInfo.parentPid == -1)
        printf("Parent PID  : (none)\n");
    else
        printf("Parent PID  : %d\n", (int) processInfo.parentPid);

    if (processInfo.waitingForPid == -1)
        printf("Waiting for : (none)\n");
    else
        printf("Waiting for : %d\n", (int) processInfo.waitingForPid);

    printf("Priority    : %d\n", (int) processInfo.priority);
    printf("RSP         : 0x%x\n", (unsigned int) processInfo.rsp);
    printf("RBP         : 0x%x\n", (unsigned int) processInfo.base);
    printf("RIP         : 0x%x\n", (unsigned int) processInfo.rip);
    printf("State       : %s\n", state);
}
  
void handle_ps(char * arg){
    uint64_t cantProcesses; 

    PCB *processInfo = syscall_getProcessInfo(&cantProcesses);

    if (cantProcesses == 0 || processInfo == NULL) {
        printf("No se encontraron procesos.\n");
        return;
    }
    
    for (int i = 0; i < cantProcesses; i++) {
        printProcessInfo(processInfo[i]);
    }

    // Solo liberamos el array de PCBs, no los campos individuales
    syscall_freeMemory(processInfo);
}

static void loop(uint64_t argc, char *argv[]) {
    pid_t pid = syscall_getpid();
    // Convertir el argumento a número
    uint32_t time = satoi(argv[0]);
    
    if (time <= 0) {
        printf("Error: time debe ser mayor que 0\n");
        return;
    }
    printf("Proceso loop %d iniciado. Saludando cada %d segundos...\n", pid, time);
    
    while(1) {
        printf("Hola! Soy el proceso %d\n", pid);
        syscall_wait(time);
    }
}

void handle_loop(char * arg) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: loop <time>\n");
        return;
    }
    char **argv = malloc(2 * sizeof(char*));
    argv[0] = arg;
    argv[1] = NULL;
    
    pid_t pid = syscall_create_process("loop", (fnptr)loop, 1, argv, 1);

    if (pid < 0) {
        printf("Error al crear el proceso de loop\n");
    }
}
