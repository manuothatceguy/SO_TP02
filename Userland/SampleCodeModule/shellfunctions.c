// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <shellfunctions.h>
#include <syscall.h>
#include <stdlib.h>
#include <test_functions.h>
#include <shared_structs.h>
#include <programs.h>
#include <shell.h>
#include "phylo.h"

#define CANT_SPECIAL_KEYS 9

extern void div_zero();
extern void invalid_opcode();

//uint64_t syscall_create_process(char *name, fnptr function, uint64_t argc, char *argv[], uint8_t priority);

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
                "    - phylo <cant_philosophers>: test de filosofos\n";


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

void handle_help(char * arg){
    free(arg); // no se usa
    printf("\n");
    printf("%s", help); 
}

void handle_time(char * arg){
    free(arg); // no se usa
    showTime();
}

void handle_registers(char * arg){
    free(arg); // no se usa
    showRegisters();
}

void handle_echo(char * arguments){
    printf("%s\n", arguments);
    free(arguments); // libero desp de usar
}

void handle_size_up(char * arg){
    free(arg); // no se usa
    syscall_sizeUpFont(1);
    syscall_clearScreen();
}

void handle_size_down(char * arg){
    free(arg); // no se usa
    syscall_sizeDownFont(1);
    syscall_clearScreen();
}

void handle_test_div_0(char * arg){
    free(arg); // no se usa
    div_zero(); 
}

void handle_test_invalid_opcode(char * arg){
    free(arg); // no se usa
    invalid_opcode(); 
}

void handle_clear(char * arg){
    free(arg); // no se usa
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
    create_process_and_wait("test_mm", (fnptr)test_mm, 1, argv, 1, 1, -1, 1);
    
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
        printf("Uso: test_sync <iterations> <use_sem>\n");
        printf("  iterations: número de iteraciones por proceso\n");
        printf("  use_sem: 1 para usar semáforos, 0 para no usarlos\n");
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
        printf("Uso: test_sync <iterations> <use_sem>\n");
        return;
    }
    
    // Crear copias de los argumentos
    char iterations[32] = {0};
    char use_sem[32] = {0};
    
    // Copiar primer argumento
    for (int i = 0; i < space_pos; i++) {
        iterations[i] = arg[i];
    }
    
    // Copiar segundo argumento
    int j = 0;
    for (int i = space_pos + 1; arg[i] != '\0'; i++) {
        use_sem[j++] = arg[i];
    }
    
    // Verificar que ambos sean números válidos
    for (int i = 0; iterations[i] != '\0'; i++) {
        if (iterations[i] < '0' || iterations[i] > '9') {
            printf("Error: iterations debe ser un número\n");
            return;
        }
    }
    
    if (use_sem[0] != '0' && use_sem[0] != '1') {
        printf("Error: use_sem debe ser 0 o 1\n");
        return;
    }
    
    printf("Iniciando test de sincronizacion...\n");
    char *argv[] = { iterations, use_sem, NULL };
    
    // Crear un nuevo proceso para ejecutar el test
    create_process_and_wait("test_sync", (fnptr)test_sync, 2, argv, 1, 1, 0, 1);
}

void handle_ps(char * arg){
    free(arg);
    uint64_t cantProcesses; 

    PCB *processInfo = syscall_getProcessInfo(&cantProcesses);

    if (cantProcesses == 0 || processInfo == NULL) {
        printf("No se encontraron procesos.\n");
        return;
    }
    
    printHeader();
    for (int i = 0; i < cantProcesses; i++) {
        printProcessInfo(processInfo[i]);
        printf("\n"); // Línea extra de separación entre procesos
    }

    syscall_freeMemory(processInfo);
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

void handle_wc(char * arg) {
    free(arg); // no se usa
    char *argv[] = { NULL };
    create_process_and_wait("wc", (fnptr)wc, 0, argv, 1, 1, 0, 1);
}

void handle_filter(char * arg) {
    free(arg); // no se usa
    char *argv[] = { NULL };
    create_process_and_wait("filter", (fnptr)filter, 0, argv, 1, 1, 0, 1);
}

void handle_cat(char * arg) {
    free(arg); // no se usa
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
    free(arg);
    
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
    free(arg); // no se usa
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
        printf("Uso: phylo <cant_philosophers>\n");
        return;
    }
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return;
    }
    argv[0] = arg;
    argv[1] = NULL;
    
    create_process_and_wait("phylo", (fnptr)phylo, 1, argv, 1, 1, 0, 1);
    free(argv);
    free(arg);
}
    