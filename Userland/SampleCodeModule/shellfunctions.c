#include "shellfunctions.h"
#include <syscall.h>
#include <stdlib.h>
#include <test_functions.h>
#include <shared_structs.h>
#include "shell.h"
#include "phylo.h"

#define CANT_REGISTERS 19
#define BUFFER_SPACE 1000
#define CANT_SPECIAL_KEYS 9

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
                "    - loop <time>: ejecuta un bucle por el tiempo especificado\n"
                "    - nice <pid> <new_prio>: cambia la prioridad de un proceso\n"
                "    - wc: cuenta la cantidad de lineas del input\n"
                "    - filter: filtra las vocales del input\n"
                "    - cat: muestra el input tal cual se ingresa\n"
                "    - test_malloc_free: test de malloc y free\n"
                "    - phylo <max_philosophers>: test de filosofos\n";


// Wrapper para crear procesos y manejar waitpid automáticamente
static pid_t create_process_and_wait(char *name, fnptr function, uint64_t argc, char **argv, uint8_t priority, char foreground, int stdin, int stdout) {
    // Crear el proceso
    pid_t pid = syscall_create_process(name, function, argc, argv, priority, foreground, stdin, stdout);
    
    if (pid < 0) {
        printf("Error al crear el proceso %s\n", name);
        return -1;
    }
    
    printf("Proceso %s creado con PID: %d\n", name, pid);
    
    // Si es foreground, esperar a que termine
    if (foreground) {
        int status = 0;
        int waited_pid = syscall_waitpid(pid, &status);
        if (waited_pid == pid) {
            printf("Proceso %s (PID: %d) termino con valor %d\n", name, pid, status);
        }
    }
    
    return pid;
}

// Función auxiliar para parsear argumentos
static int parse_arguments(char *arg, char **args, int max_args, int max_size) {
    if (arg == NULL || arg[0] == '\0') {
        return -1;
    }
    
    int arg_count = 0;
    int current_pos = 0;
    int start_pos = 0;
    
    // Inicializar todos los argumentos como strings vacíos
    for (int i = 0; i < max_args; i++) {
        args[i][0] = '\0';
    }
    
    // Procesar cada carácter
    while (arg[current_pos] != '\0' && arg_count < max_args) {
        // Si encontramos un espacio o el final de la cadena
        if (arg[current_pos] == ' ' || arg[current_pos + 1] == '\0') {
            // Si es el final de la cadena, incluir el último carácter
            int end_pos = (arg[current_pos + 1] == '\0') ? current_pos + 1 : current_pos;
            
            // Copiar el argumento actual
            int j = 0;
            for (int i = start_pos; i < end_pos && j < max_size - 1; i++) {
                args[arg_count][j++] = arg[i];
            }
            args[arg_count][j] = '\0';
            
            // Solo incrementar arg_count si el argumento no está vacío
            if (j > 0) {
                arg_count++;
            }
            
            start_pos = current_pos + 1;
        }
        current_pos++;
    }
    
    return arg_count;
}

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
    
    // Crear un nuevo proceso para ejecutar el test (background porque es test)
    create_process_and_wait("test_mm", (fnptr)test_mm, 1, argv, 1, 0, 0, 1);
    
    // Liberar la memoria después de crear el proceso
    //free(argv);
}

void handle_test_processes(char * arg) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_processes <max_processes>\n");
        return;
    }
    printf("Iniciando test de procesos...\n");
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return;
    }
    argv[0] = arg; // Asignar el argumento
    argv[1] = NULL; // Terminar el array de argumentos
    
    create_process_and_wait("test_processes", (fnptr)test_processes, 1, argv, 1, 0, 0, 1);
    
    // Liberar la memoria después de crear el proceso
    //free(argv);
}

void handle_test_prio(char * arg) {
    printf("Iniciando test de prioridades...\n");
    char *argv[] = { NULL };
    
    // Crear un nuevo proceso para ejecutar el test
    create_process_and_wait("test_prio", (fnptr)test_prio, 0, argv, 1, 1, 0, 1);
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
    create_process_and_wait("test_sync", (fnptr)test_sync, 2, argv, 1, 1, 0, 1);
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
    if (arg == NULL || checkNumber(arg) == 0) {
        printf("Uso: loop <time>\n");
        return;
    }
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return;
    }
    argv[0] = arg;
    argv[1] = NULL;
    
    create_process_and_wait("loop", (fnptr)loop, 1, argv, 1, 1, -1, 1);
    
    // Liberar memoria
    //free(argv);
}

static void cat(uint64_t argc, char *argv[]) {
    char c;
    char buffer[BUFFER_SPACE] = {0};
    int i = 0;
    
    //printf("Ingrese el texto (presione Ctrl+D para terminar):\n");
    
    c = getChar();
    while (c != EOF) {
        if (c != 0) {
            printf("%c", c);
            buffer[i++] = c;
            if (c == '\n') {
                buffer[i] = '\0';
                printf("%s", buffer);
                buffer[0] = '\0';
                i = 0;
            }
        }
        c = getChar();
    }
    printf("\n");
}

static void wc(uint64_t argc, char *argv[]) {
    int lines = 1;
    char c;
    
    //printf("Ingrese el texto (presione Ctrl+D para terminar):\n");
    
    c = getChar();
    while (c != EOF) {
        if (c != 0) {
            if (c == '\n') {
                lines++;
            }
            printf("%c", c);
        }
        c = getChar();
    }
    printf("\n");
    
    printf("Cantidad de lineas: %d\n\n", --lines);
}

static void filter(uint64_t argc, char *argv[]) {
    char c;
    char filtered[BUFFER_SPACE] = {0};
    int i = 0;
    
    //printf("Ingrese el texto (presione Ctrl+D para terminar):\n");
    
    c = getChar();
    while (c != EOF) {
        if (c != 0) {
            printf("%c", c);
            if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
                c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U') {
                filtered[i++] = c;
            }
        }
        c = getChar();
    }
    printf("\n%s\n", filtered);
}

void handle_wc(char * arg) {
    char *argv[] = { NULL };
    create_process_and_wait("wc", (fnptr)wc, 0, argv, 1, 1, 0, 1);
}

void handle_filter(char * arg) {
    char *argv[] = { NULL };
    create_process_and_wait("filter", (fnptr)filter, 0, argv, 1, 1, 0, 1);
}

void handle_cat(char * arg) {
    char *argv[] = { NULL };
    create_process_and_wait("cat", (fnptr)cat, 0, argv, 1, 1, 0, 1);
}

void handle_nice(char * arg) {
    char *args[2];
    char arg1[32] = {0};
    char arg2[32] = {0};
    args[0] = arg1;
    args[1] = arg2;
    
    if (parse_arguments(arg, args, 2, 32) != 2) {
        printf("Uso: nice <pid> <new_priority>\n");
        return;
    }
    
    // Convertir strings a números
    int pid = satoi(args[0]);
    int new_priority = satoi(args[1]);
    
    // Verificar que ambos sean números válidos
    if (pid == 0) {
        printf("Error: la shell no se toca\n");
        return;
    }else if (pid < 0) {
        printf("Error: pid debe ser un número positivo\n");
        return;
    }else if (new_priority < 0 || new_priority > 5) {
        printf("Error: la prioridad debe estar entre 0 y 5\n");
        return;
    }
    
    // Llamar a la syscall para cambiar la prioridad
    if (syscall_changePrio(pid, new_priority) == -1) {
        printf("Error al cambiar la prioridad del proceso %d\n", pid);
    } else {
        printf("Prioridad del proceso %d cambiada a %d\n", pid, new_priority);
    }
}

void handle_test_malloc_free(char *arg) {
    printf("Estado de memoria antes de malloc:\n");
    memInfo info;
    syscall_memInfo(&info);
    printf("Usada: %d, Libre: %d\n", info.used, info.free);

    uint64_t size = 1000;
    void *ptr = syscall_allocMemory(size);
    if (!ptr) {
        printf("Fallo el malloc\n");
        return;
    }
    printf("Estado de memoria después de malloc:\n");
    syscall_memInfo(&info);
    printf("Usada: %d, Libre: %d\n", info.used, info.free);

    syscall_freeMemory(ptr);

    printf("Estado de memoria después de free:\n");
    syscall_memInfo(&info);
    printf("Usada: %d, Libre: %d\n", info.used, info.free);
}

void handle_phylo(char * arg) {
    if ( arg == NULL || arg[0] == '\0') {
        printf("Uso: phylo <max_philosophers>\n");
        return;
    }
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return;
    }
    argv[0] = arg;
    argv[1] = NULL;
    
    syscall_create_process("phylo", (fnptr)phylo, 1, argv, 1, 1, 0, 1);
}
    