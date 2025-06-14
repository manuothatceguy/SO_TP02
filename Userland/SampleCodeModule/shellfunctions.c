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
                "    - phylo <cant_philosophers>: test de filosofos\n"
                "    - kill <pid>: mata el proceso con el pid especificado\n"
                "    - block <pid>: bloquea el proceso con el pid especificado\n"
                "    - unblock <pid>: desbloquea el proceso con el pid especificado\n";

void clearBuffer(){
    syscall_clear_pipe(0);
}

uint64_t doHelp(uint64_t argc, char ** argv){
    printf("\n%s", help);
    return 0;
}

pid_t handle_help(char * arg, int stdin, int stdout){
    return syscall_create_process("help", (fnptr)doHelp, NULL, 1, 1, stdin, stdout);
}

uint64_t echo(int argc, char ** argv){
    if(argc == 0){
        return 1;
    }
    printf("%s\n", *argv);
    return 0;
}

pid_t handle_echo(char * arg, int stdin, int stdout){
    return syscall_create_process("echo", (fnptr)echo, &arg, 1, 1, -1, stdout);
}

uint64_t clearScreen(int argc, char ** argv){
    syscall_clearScreen();
    return 0;
}

pid_t handle_clear(char * arg, int stdin, int stdout){
    free(arg); // no se usa
    return syscall_create_process("clear", (fnptr)clearScreen, NULL, 1, 1, stdin, stdout);
}

uint64_t showMemInfo(int argc, char ** argv){
    printf("Estado de memoria:\n");

    memInfo info;
    if (syscall_memInfo(&info) == -1) {
        printferror("Error al obtener el estado de memoria\n");
        return 1;
    }
    
    printf("Memoria total: %l bytes\n", info.total);
    printf("Memoria usada: %l bytes\n", info.used);
    printf("Memoria libre: %l bytes\n", info.free);
    return 0;
}

pid_t handle_mem_info(char * arg, int stdin, int stdout) {
    return syscall_create_process("memInfo", (fnptr)showMemInfo, NULL, 1, 1, stdin, stdout);
}

pid_t handle_test_mm(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 1, 32);
    if (bg == -1) {
        printferror("Uso: test_mm <max_memory>\n");
        return -1;
    }
    printf("Iniciando test de gestion de memoria...\n");
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printf("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = args[0];
    argv[1] = NULL;
    
    pid_t pid = syscall_create_process("test_mm", (fnptr)test_mm, argv, 1, !bg, -1, stdout);
    free(argv);
    return !bg ? pid : 0;
}

pid_t handle_test_processes(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 1, 32);
    if (bg == -1) {
        printferror("Uso: test_processes <max_processes>\n");
        return -1;
    }
    printf("Iniciando test de procesos...\n");
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printferror("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = args[0];
    argv[1] = NULL;
    
    pid_t pid = syscall_create_process("test_processes", (fnptr)test_processes, argv, 1, !bg, -1, stdout);
    free(argv);
    return !bg ? pid : 0;
}

pid_t handle_test_prio(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 0, 32);
    if (bg == -1) {
        printferror("Uso: test_prio\n");
        return -1;
    }
    pid_t pid = syscall_create_process("test_prio", (fnptr)test_prio, NULL, 1, 1, -1, stdout);
    return !bg ? pid : 0;
}

pid_t handle_test_sync(char * arg, int stdin, int stdout) {
    char *args[2];
    int bg = anal_arg(arg, args, 2, 32);
    if (bg == -1) {
        printferror("Uso: test_sync <iterations> <use_sem>\n");
        printferror("  iterations: numero de iteraciones por proceso\n");
        printferror("  use_sem: 1 para usar semaforos, 0 para no usarlos\n");
        return -1;
    }
    
    printf("Iniciando test de sincronizacion...\n");
    char **argv = malloc(3 * sizeof(char*));

    if (argv == NULL) {
        printferror("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = args[0];
    argv[1] = args[1];
    argv[2] = NULL;
    pid_t pid = syscall_create_process("test_sync", (fnptr)test_sync, argv, 1, !bg, -1, stdout);
    free(argv); //eureka
    return !bg ? pid : 0;
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
        printf("\n"); 
    }

    syscall_freeMemory(processInfo);
    return 0;
}

pid_t handle_ps(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 0, 32);
    if (bg == -1) {
        printferror("Uso: ps\n");
        return -1;
    }
    pid_t pid = syscall_create_process("ps", (fnptr)processInfo, NULL, 1, 1, -1, stdout);
    return !bg ? pid : 0;
}


pid_t handle_loop(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 1, 32);
    if (bg == -1) {
        printferror("Uso: loop <time>\n");
        return -1;
    }

    pid_t pid = syscall_create_process("loop", (fnptr)loop, args, 1, !bg, -1, stdout);
    return !bg ? pid : 0;
}

pid_t handle_wc(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 0, 32);
    if (bg == -1) {
        printferror("Uso: wc\n");
        return -1;
    }
    pid_t pid = syscall_create_process("wc", (fnptr)wc, NULL, 1, 1, stdin, stdout);
    return !bg ? pid : 0;
}

pid_t handle_filter(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 0, 32);
    if (bg == -1) {
        printferror("Uso: filter\n");
        return -1;
    }
    pid_t pid = syscall_create_process("filter", (fnptr)filter, NULL, 1, 1, stdin, stdout);
    return !bg ? pid : 0;
}

pid_t handle_cat(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 0, 32);
    if (bg == -1) {
        printferror("Uso: cat\n");
        return -1;
    }
    pid_t pid = syscall_create_process("cat", (fnptr)cat, NULL, 1, !bg, stdin, stdout);

    return !bg ? pid : 0;
}

uint64_t nice(int argc, char **argv){
    
    uint64_t pid = satoi(argv[0]);
    int8_t new_priority = satoi(argv[1]);
    
    if (pid == 0) {
        printferror("Error: la shell no se toca\n");
        return 1;
    } else if (new_priority < 0 || new_priority > 5) {
        printferror("Error: la prioridad debe estar entre 0 y 5\n");
        return 1;
    }
    
    if (syscall_changePrio(pid, new_priority) == -1) {
        printferror("Error al cambiar la prioridad del proceso %l\n", pid);
    } else {
        printf("Prioridad del proceso %l cambiada a %d\n", pid, new_priority);
    }
    return 0;
}

pid_t handle_nice(char * arg, int stdin, int stdout) {
    char *args[2];
    int bg = anal_arg(arg, args, 2, 32);
    if (bg == -1) {
        printferror("Uso: nice <pid> <new_priority>\n");
        return -1;
    }
    char **argv = malloc(3 * sizeof(char*));
    if (argv == NULL) {
        printferror("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = args[0];
    argv[1] = args[1];
    argv[2] = NULL;

    pid_t pid = syscall_create_process("nice", (fnptr)nice, argv, 1, !bg, -1, stdout);
    return !bg ? pid : 0;
}

uint64_t test_malloc_free(int argc, char** argv){
    printf("Estado de memoria antes de malloc:\n");
    memInfo info;
    syscall_memInfo(&info);
    printf("Usada: %l, Libre: %l\n", info.used, info.free);

    uint64_t size = 1000;
    void *ptr = syscall_allocMemory(size);
    if (!ptr) {
        printferror("Error al reservar memoria\n");
        return 1;
    }
    printf("Estado de memoria despues de malloc:\n");
    syscall_memInfo(&info);
    printf("Usada: %l, Libre: %l\n", info.used, info.free);

    syscall_freeMemory(ptr);

    printf("Estado de memoria despues de free:\n");
    syscall_memInfo(&info);
    printf("Usada: %l, Libre: %l\n", info.used, info.free);
    return 0;
}

pid_t handle_test_malloc_free(char *arg, int stdin, int stdout) {
    pid_t pid = syscall_create_process("test_malloc_free", (fnptr)test_malloc_free, NULL, 1, 1, -1, stdout);
    printf("Proceso %d ejecutado en background.\n", pid);
    return pid;
}

pid_t handle_phylo(char * arg, int stdin, int stdout) {
    char *args[1];
    int bg = anal_arg(arg, args, 1, 32);
    if (bg == -1) {
        printferror("Uso: phylo <cant_philosophers>\n");
        return -1;
    }
    char **argv = malloc(2 * sizeof(char*));
    if (argv == NULL) {
        printferror("Error al asignar memoria para los argumentos\n");
        return -1;
    }
    argv[0] = args[0];
    argv[1] = NULL;
    
    pid_t pid = syscall_create_process("phylo", (fnptr)phylo, argv, 1, !bg, stdin, stdout);
    free(argv);
    return !bg ? pid : 0;
}

char checkErrorInPid(uint64_t pid) {
    if (pid == 1) {
        printferror("Error: la shell no se toca\n");
    } else if(pid == 0) {
        printferror("Error: idle no se toca\n");
    }
    return (pid <= 1);
}

void kill(char * arg){
    uint64_t pid = satoi(arg);
    if(checkErrorInPid(pid)) {
        return;
    }

    if (syscall_kill(pid) == -1) {
        printferror("Error al matar el proceso %l\n", pid);
    } else {
        printf("Proceso %l terminado correctamente\n", pid);
    }
}

void block(char * arg){
    uint64_t pid = satoi(arg);
    if(checkErrorInPid(pid)) {
        return;
    }

    if (syscall_block(pid) == -1) {
        printferror("Error al bloquear el proceso %l\n", pid);
    } else {
        printf("Proceso %l bloqueado correctamente\n", pid);
    }
}

void unblock(char * arg){
    uint64_t pid = satoi(arg);
    if(checkErrorInPid(pid)) {
        return;
    }

    if (syscall_unblock(pid) == -1) {
        printferror("Error al desbloquear el proceso %l\n", pid);
    } else {
        printf("Proceso %l desbloqueado correctamente\n", pid);
    }
}