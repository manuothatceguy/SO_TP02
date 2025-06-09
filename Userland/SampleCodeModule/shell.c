// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shellfunctions.h"
#include "shell.h"
#include <stdlib.h>
#include "syscall.h"
#include <stdint.h>
#include <programs.h>


#define FALSE 0
#define TRUE 1
#define MAX_ECHO 1000
#define MAX_USERNAME_LENGTH 16
#define PROMPT "%s$> "
#define CANT_INSTRUCTIONS 21
uint64_t curr = 0;

typedef enum {
    HELP = 0,
    ECHO, 
    CLEAR,
    TEST_MM,
    TEST_PROCESSES,
    TEST_PRIO,
    TEST_SYNC,
    PS,
    MEM_INFO,
    LOOP,
    NICE,
    WC,
    FILTER,
    CAT,
    TEST_MALLOC_FREE,
    PHYLO,
    KILL,
    BLOCK,
    UNBLOCK,
    EXIT
} instructions;

#define IS_BUILT_IN(i) ((i) >= KILL && (i) < EXIT)

static char * inst_list[] = {               "help", 
                                            "echo", 
                                            "clear",
                                            "test_mm",
                                            "test_processes",
                                            "test_prio",
                                            "test_sync",
                                            "ps",
                                            "memInfo",
                                            "loop",
                                            "nice",
                                            "wc",
                                            "filter",
                                            "cat",
                                            "test_malloc_free",
                                            "phylo",
                                            "kill",
                                            "block",
                                            "unblock",
                                            "exit",
                            };

pid_t (*instruction_handlers[CANT_INSTRUCTIONS-4])(char *, int, int) = {
    handle_help,
    handle_echo,
    handle_clear,
    handle_test_mm,
    handle_test_processes,
    handle_test_prio,
    handle_test_sync,
    handle_ps,
    handle_mem_info,
    handle_loop,
    handle_nice,
    handle_wc,
    handle_filter,
    handle_cat,
    handle_test_malloc_free,
    handle_phylo,
};

void (*built_in_handlers[(EXIT-KILL)])(char *) = {
    kill,
    block,
    unblock,
};

int verifyInstruction(char * instruction){
    for(int i = 0; i < CANT_INSTRUCTIONS-1; i++){
        if(strcmp(inst_list[i], instruction) == 0){
            return i;
        }
    }
    return -1;
}


int getInstruction(char * buffer, char * arguments){
    int i = 0;
    int j = 0;

    char * instruction = malloc(BUFFER_SPACE * sizeof(char));
    if(instruction == NULL) {
        printferror("Error al asignar memoria para la instruccion.\n");
        return -1;
    }

    for(; i < BUFFER_SPACE; i++){
        if(buffer[i] == ' ' || buffer[i] == '\0'){
            instruction[j] = 0;
            i++;
            break;
        }
        else {
            instruction[j++] = buffer[i];
        }
    }

    int arg_index = 0;
    while (buffer[i] != '\0' && buffer[i] != '\n') {
        arguments[arg_index++] = buffer[i++];
    }
    arguments[arg_index] = '\0';
    free(buffer);

    int iNum = 0;
    if((iNum = verifyInstruction(instruction)) == -1 && instruction[0] != 0){
        printferror("Comando no reconocido: %s\n", instruction);
    }
    free(instruction);
    return iNum;
}

static int bufferControl(pipeCmd * pipe_cmd){
    char * shell_buffer = malloc(BUFFER_SPACE * sizeof(char));
    if(shell_buffer == NULL) {
        printferror("Error al asignar memoria para el buffer.\n");
        return -1;
    }

    int instructions = 0;
    readLine(shell_buffer, BUFFER_SPACE);

    char * pipe_pos = strstr(shell_buffer, "|");
    if(pipe_pos != NULL){
        *(pipe_pos-1) = 0;
        *pipe_pos = 0;
        *(pipe_pos+1) = 0;
        char * arg2 = malloc(BUFFER_SPACE * sizeof(char));
        pipe_cmd->cmd2.instruction = getInstruction(pipe_pos+2, arg2);
        pipe_cmd->cmd2.arguments = arg2;
        if(pipe_cmd->cmd2.instruction >= 0) instructions++;
    }
    char * arg1 = malloc(BUFFER_SPACE * sizeof(char));
    pipe_cmd->cmd1.instruction = getInstruction(shell_buffer, arg1);
    pipe_cmd->cmd1.arguments = arg1;
    if(pipe_cmd->cmd1.instruction >= 0) instructions++;
    if(IS_BUILT_IN(pipe_cmd->cmd1.instruction) && IS_BUILT_IN(pipe_cmd->cmd2.instruction)){
        printferror("No se pueden usar comandos built-in con pipes.\n");
        free(pipe_cmd->cmd1.arguments);
        free(pipe_cmd->cmd2.arguments);
        free(pipe_cmd);
        return -1;
    }
    return instructions;
}

static void handle_piped_commands(pipeCmd * pipe_cmd){
    if(pipe_cmd->cmd1.instruction == -1 || pipe_cmd->cmd2.instruction == -1){
        printferror("Comando invalido.\n");
        free(pipe_cmd->cmd1.arguments);
        free(pipe_cmd->cmd2.arguments);
        free(pipe_cmd);
        return;
    }
    if(IS_BUILT_IN(pipe_cmd->cmd1.instruction) || IS_BUILT_IN(pipe_cmd->cmd2.instruction)){
        printferror("No se pueden usar comandos built-in con pipes.\n");
        free(pipe_cmd->cmd1.arguments);
        free(pipe_cmd->cmd2.arguments);
        free(pipe_cmd);
        return;
    }

    int pipe = syscall_open_pipe();
    if(pipe < 0){
        printferror("Error al abrir el pipe\n");
        return;
    }

    pid_t pids[2];
    pids[0] = instruction_handlers[pipe_cmd->cmd1.instruction](pipe_cmd->cmd1.arguments, 0, pipe);
    pids[1] = instruction_handlers[pipe_cmd->cmd2.instruction](pipe_cmd->cmd2.arguments, pipe, 1);
    int status = 0;
    syscall_waitpid(pids[0], &status);
    free(pipe_cmd->cmd1.arguments);
    printf("Proceso %d terminado con estado %d\n", pids[0], status);
    syscall_waitpid(pids[1], &status);
    free(pipe_cmd->cmd2.arguments);
    printf("Proceso %d terminado con estado %d\n", pids[1], status);
    free(pipe_cmd);
    syscall_close_pipe(pipe);
}

uint64_t shell(uint64_t argc, char **argv) {
    syscall_clearScreen();
    syscall_sizeUpFont(1);
    printf("  Bienvenido a la shell\n\n");
    syscall_sizeDownFont(1);

    printf("\n\n\n");
    printf("  Ingrese 'help' para ver la lista de comandos disponibles.\n");
    printf("  Ingrese 'exit' para salir de la shell.\n\n");

    printf("Ingrese su nombre de usuario: ");
    char username[MAX_USERNAME_LENGTH];
    readLine(username, MAX_USERNAME_LENGTH);

    unsigned int exit = FALSE;
    int instructions;
    syscall_clearScreen();
    pipeCmd * pipe_cmd;
    while(!exit){
        clearBuffer();
        printf(PROMPT, username);
        pipe_cmd = (pipeCmd *)malloc(sizeof(pipeCmd));
        if(pipe_cmd == NULL){
            printferror("Error al asignar memoria para los argumentos.\n");
            return 1;
        }
        pipe_cmd->cmd1 = (command){-1,0};
        pipe_cmd->cmd2 = (command){-1,0};
        instructions =  bufferControl(pipe_cmd);
        switch (instructions) {
            case 0: // built-in
                if (pipe_cmd->cmd1.instruction == -1) {
                        printferror("Comando invalido.\n");
                        free(pipe_cmd->cmd1.arguments);
                        free(pipe_cmd);
                }else if(IS_BUILT_IN(pipe_cmd->cmd1.instruction)) {
                    built_in_handlers[pipe_cmd->cmd1.instruction - KILL](pipe_cmd->cmd1.arguments);
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                }   
                break;
            case 1:
                if (pipe_cmd->cmd1.instruction == EXIT) {
                    exit = TRUE;
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                    break;
                } else {
                    pid_t pid = instruction_handlers[pipe_cmd->cmd1.instruction](pipe_cmd->cmd1.arguments, 0, 1);
                    if (pid < 0) {
                        printferror("Error al ejecutar el comando.\n");
                    } else if (pid == 0) {
                        printf("Proceso %s ejecutado en background.\n", inst_list[pipe_cmd->cmd1.instruction]);
                    } else {
                        int status = 0;
                        syscall_waitpid(pid, &status);
                        printf("Proceso %d terminado con estado %d\n", pid, status);
                    }
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                }
                break;
            case 2:
                handle_piped_commands(pipe_cmd);
                break;
            
            default:
                break;
            
        }
    }
    printf("Saliendo de la terminal...\n");
    syscall_wait(3);
    syscall_clearScreen();
    printf("\n\n\n\n");
    syscall_sizeUpFont(1);
    printf("Adios %s, gracias por usar la shell!\n", username);
    syscall_sizeDownFont(1);
    printf("\n\n    \"No llores porque termino, sonrie porque sucedio.\"\n\n      Dr. Seuss.");
    syscall_wait(200);
    syscall_clearScreen();
    return 0;
}
