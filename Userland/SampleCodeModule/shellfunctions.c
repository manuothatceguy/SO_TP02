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


char * help =   " Lista de comandos disponibles:\n"        
                "    - exit: corta la ejecucion\n"
                "    - help: muestra este menu\n"
                "    - echo: imprime lo que le sigue a echo\n"
                "    - clear: borra la pantalla y comienza arriba\n"
                "    - test_mm <max_memory>: test de gestion de memoria\n"
                "    - test_processes <max_processes>: test de procesos\n"
                "    - test_prio: test de prioridades\n"
                "    - test_sync <iterations> <use_sem>: test de sincronizacion\n"
                "    - ps: muestra los procesos con su informacion\n"
                "    - memInfo: imprime estado de la memoria\n"
                "    - loop <time>: ejecuta un bucle por el tiempo especificado\n"
                "    - nice <pid> <new_prio>: cambia la prioridad de un proceso\n"
                "    - wc: cuenta la cantidad de lineas del input\n"
                "    - filter: filtra las vocales del input\n"
                "    - cat: muestra el input tal cual se ingresa\n"
                "    - test_malloc_free: test de malloc y free\n"
                "    - phylo <cant_philosophers>: test de filosofos\n";

uint64_t doHelp(uint64_t argc, char ** argv){
    printf("\n%s", help);
    return 0;
}

pid_t handle_help(char * arg, int stdin, int stdout){
    return syscall_create_process("help", (fnptr)doHelp, 0, NULL, 1, 1, stdin, stdout);
}

uint64_t echo(int argc, char ** argv){
    if(argc == 0){
        return 1;
    }
    printf("%s\n", *argv);
    return 0;
}

pid_t handle_echo(char * arg, int stdin, int stdout){
    return syscall_create_process("echo", (fnptr)echo, 1, &arg, 1, 1, stdin, stdout);
}

uint64_t clearScreen(int argc, char ** argv){
    syscall_clearScreen();
    return 0;
}

pid_t handle_clear(char * arg, int stdin, int stdout){
    free(arg); // no se usa
    return syscall_create_process("clear", (fnptr)clearScreen, 0, NULL, 1, 1, stdin, stdout);
}

uint64_t showMemInfo(int argc, char ** argv){
    printf("Estado de memoria:\n");

    memInfo info;
    if (syscall_memInfo(&info) == -1) {
        printf("Error al obtener el estado de memoria\n");
        return 1;
    }
    
    printf("Memoria total: %d bytes\n", info.total);
    printf("Memoria usada: %d bytes\n", info.used);
    printf("Memoria libre: %d bytes\n", info.free);
    return 0;
}

pid_t handle_mem_info(char * arg, int stdin, int stdout) {
    return syscall_create_process("memInfo", (fnptr)showMemInfo, 0, NULL, 1, 1, stdin, stdout);
}

pid_t handle_test_mm(char * arg, int stdin, int stdout) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_mm <max_memory>\n");
        return -1;
    }
    printf("Iniciando test de gestion de memoria...\n");
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = arg; // Asignar el argumento
    argv[1] = NULL; // Terminar el array de argumentos
    
    // Crear un nuevo proceso para ejecutar el test (background porque es test)
    return syscall_create_process("test_mm", (fnptr)test_mm, 1, argv, 1, 1, -1, stdout);
    
    // Liberar la memoria después de crear el proceso
    free(argv);
}

pid_t handle_test_processes(char * arg, int stdin, int stdout) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_processes <max_processes>\n");
        return -1;
    }
    printf("Iniciando test de procesos...\n");
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = arg; // Asignar el argumento
    argv[1] = NULL; // Terminar el array de argumentos
    
    return syscall_create_process("test_processes", (fnptr)test_processes, 1, argv, 1, 1, stdin, stdout);
    
    // Liberar la memoria después de crear el proceso
    free(argv);
}

pid_t handle_test_prio(char * arg, int stdin, int stdout) {
    return syscall_create_process("test_prio", (fnptr)test_prio, 0, NULL, 1, 1, stdin, stdout);
}

pid_t handle_test_sync(char * arg, int stdin, int stdout) {
    if (arg == NULL || arg[0] == '\0') {
        printf("Uso: test_sync <iterations> <use_sem>\n");
        printf("  iterations: número de iteraciones por proceso\n");
        printf("  use_sem: 1 para usar semáforos, 0 para no usarlos\n");
        return -1;
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
        return -1;
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
            return -1;
        }
    }
    
    if (use_sem[0] != '0' && use_sem[0] != '1') {
        printf("Error: use_sem debe ser 0 o 1\n");
        return -1;
    }
    
    printf("Iniciando test de sincronizacion...\n");
    char *argv[] = { iterations, use_sem, NULL };
    
    // Crear un nuevo proceso para ejecutar el test
    return syscall_create_process("test_sync", (fnptr)test_sync, 2, argv, 1, 1, stdin, stdout);
}

uint64_t processInfo(uint64_t argc, char ** argv) {
    uint64_t cantProcesses; 

    PCB *processInfo = syscall_getProcessInfo(&cantProcesses);

    if (cantProcesses == 0 || processInfo == NULL) {
        printf("No se encontraron procesos.\n");
        return -1;
    }
    
    printHeader();
    for (int i = 0; i < cantProcesses; i++) {
        printProcessInfo(processInfo[i]);
        printf("\n"); // Línea extra de separación entre procesos
    }

    syscall_freeMemory(processInfo);
    return 0;
}

pid_t handle_ps(char * arg, int stdin, int stdout) {
    return syscall_create_process("ps", (fnptr)processInfo, 0, NULL, 1, 1, stdin, stdout);
}


pid_t handle_loop(char * arg, int stdin, int stdout) {
    if (arg == NULL || checkNumber(arg) == 0) {
        printf("Uso: loop <time>\n");
        return -1;
    }
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = arg;
    argv[1] = NULL;
    
    pid_t pid = syscall_create_process("loop", (fnptr)loop, 1, argv, 1, 1, -1, stdout);
    free(argv); // Liberar memoria después de crear el proceso
    return pid;
}

pid_t handle_wc(char * arg, int stdin, int stdout) {
    return syscall_create_process("wc", (fnptr)wc, 0, NULL, 1, 1, stdin, stdout);
}

pid_t handle_filter(char * arg, int stdin, int stdout) {
    return syscall_create_process("filter", (fnptr)filter, 0, NULL, 1, 1, stdin, stdout);
}

pid_t handle_cat(char * arg, int stdin, int stdout) {
    return syscall_create_process("cat", (fnptr)cat, 0, NULL, 1, 1, stdin, stdout);
}

uint64_t nice(int argc, char **argv){
    
    // Convertir strings a números
    int pid = satoi(argv[0]);
    int new_priority = satoi(argv[1]);
    
    // Verificar que ambos sean números válidos
    if (pid == 0) {
        printf("Error: la shell no se toca\n");
        return 1;
    }else if (pid < 0) {
        printf("Error: pid debe ser un número positivo\n");
        return 1;
    }else if (new_priority < 0 || new_priority > 5) {
        printf("Error: la prioridad debe estar entre 0 y 5\n");
        return 1;
    }
    
    // Llamar a la syscall para cambiar la prioridad
    if (syscall_changePrio(pid, new_priority) == -1) {
        printf("Error al cambiar la prioridad del proceso %d\n", pid);
    } else {
        printf("Prioridad del proceso %d cambiada a %d\n", pid, new_priority);
    }
    return 0;
}

pid_t handle_nice(char * arg, int stdin, int stdout) {
    char *args[2];
    char arg1[32] = {0};
    char arg2[32] = {0};
    args[0] = arg1;
    args[1] = arg2;
    
    if (parse_string(arg, args, 2, 32) != 2) {
        printf("Uso: nice <pid> <new_priority>\n");
        return -1;
    }
    free(arg);
    return syscall_create_process("nice", (fnptr)nice, 2, args, 1, 1, stdin, stdout);
}

uint64_t test_malloc_free(int argc, char** argv){
    printf("Estado de memoria antes de malloc:\n");
    memInfo info;
    syscall_memInfo(&info);
    printf("Usada: %d, Libre: %d\n", info.used, info.free);

    uint64_t size = 1000;
    void *ptr = syscall_allocMemory(size);
    if (!ptr) {
        printf("Fallo el malloc\n");
        return 1;
    }
    printf("Estado de memoria después de malloc:\n");
    syscall_memInfo(&info);
    printf("Usada: %d, Libre: %d\n", info.used, info.free);

    syscall_freeMemory(ptr);

    printf("Estado de memoria después de free:\n");
    syscall_memInfo(&info);
    printf("Usada: %d, Libre: %d\n", info.used, info.free);
    return 0;
}

pid_t handle_test_malloc_free(char *arg, int stdin, int stdout) {
    return syscall_create_process("test_malloc_free", (fnptr)test_malloc_free, 0, NULL, 1, 1, stdin, stdout);
}

pid_t handle_phylo(char * arg, int stdin, int stdout) {
    if ( arg == NULL || arg[0] == '\0') {
        printf("Uso: phylo <cant_philosophers>\n");
        return -1;
    }
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return -1;
    }

    argv[0] = arg;
    argv[1] = NULL;
    
    pid_t pid = syscall_create_process("phylo", (fnptr)phylo, 1, argv, 1, 1, stdin, stdout);
    free(argv);
    return pid;
}