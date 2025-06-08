#include "shellfunctions.h"
#include "shell.h"
#include <stdlib.h>
#include "syscall.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1
#define MAX_ECHO 1000
#define MAX_USERNAME_LENGTH 16
#define PROMPT "%s$> "
#define CANT_INSTRUCTIONS 23
uint64_t curr = 0;

typedef enum {
    HELP = 0,
    TIME,
    REGISTERS,
    ECHO, 
    SIZE_UP,
    SIZE_DOWN,
    TEST_DIV_0,
    TEST_INVALID_OPCODE,
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
    EXIT
} instructions;

static char * inst_list[] = {"help", 
                                            "time", 
                                            "registers", 
                                            "echo", 
                                            "size_up",
                                            "size_down",
                                            "test_div_0", 
                                            "test_invalid_opcode", 
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
                                            "exit"
                                            };

void (*instruction_handlers[CANT_INSTRUCTIONS-1])(char *) = {
    handle_help,
    handle_time,
    handle_registers,
    handle_echo,
    handle_size_up,
    handle_size_down,
    handle_test_div_0,
    handle_test_invalid_opcode,
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
    handle_phylo
};

static void clearBuffer(){
    syscall_clear_pipe(0);
}

int verifyInstruction(char * instruction){
    for(int i = 0; i < CANT_INSTRUCTIONS; i++){
        if(strcmp(inst_list[i], instruction) == 0){
            return i;
        }
    }
    return -1;
}

int getInstruction(char * arguments){
    char * shell_buffer = malloc(BUFFER_SPACE * sizeof(char));
    if(shell_buffer == NULL) {
        printferror("Error al asignar memoria para el buffer.\n");
        return -1;
    }
    
    readLine(shell_buffer, BUFFER_SPACE);
    int i = 0;
    int j = 0;

    char * instruction = malloc(BUFFER_SPACE * sizeof(char));
    if(instruction == NULL) {
        printferror("Error al asignar memoria para la instrucción.\n");
        free(shell_buffer);
        return -1;
    }

    for(; i < BUFFER_SPACE; i++){
        if(shell_buffer[i] == ' ' || shell_buffer[i] == '\0'){
            instruction[j] = 0;
            i++;
            break;
        }
        else {
            instruction[j++] = shell_buffer[i];
        }
    }

    int arg_index = 0;

    while (shell_buffer[i] != '\0' && shell_buffer[i] != '\n') {
        arguments[arg_index++] = shell_buffer[i++];
    }
    arguments[arg_index] = '\0';
    free(shell_buffer);

    int iNum = 0;
    if((iNum = verifyInstruction(instruction)) == -1 && instruction[0] != 0){
        printferror("Comando no reconocido: %s\n", instruction);
        free(arguments);
    }
    free(instruction);
    return iNum;
}

uint64_t shell(uint64_t argc, char **argv) {
    syscall_clearScreen();
    syscall_sizeUpFont(1);
    printf("  Bienvenido a la shell\n\n");
    syscall_sizeDownFont(1);

    handle_help(0);

    printf("Ingrese su nombre de usuario: ");
    char username[MAX_USERNAME_LENGTH];
    readLine(username, MAX_USERNAME_LENGTH);

    unsigned int exit = FALSE;
    int instruction;
    syscall_clearScreen();
    char *arg;
    while(!exit){
        clearBuffer();
        printf(PROMPT, username);
        arg = (char *)malloc(BUFFER_SPACE * sizeof(char));
        if(arg == NULL){
            printferror("Error al asignar memoria para los argumentos.\n");
            return 1;
        }
        for(int i = 0; i < BUFFER_SPACE; i++) {
            arg[i] = 0; // Inicializa el buffer de argumentos
        }
        instruction = getInstruction(arg); // Lee el comando que ingresa el usuario en la shell
        if(instruction != -1){
            if(instruction != EXIT){
                instruction_handlers[instruction](arg);
            } else {
                exit = TRUE;
            }
        }
        
    }
    printf("Saliendo de la terminal...\n");
    syscall_wait(2000);
    return 0;
}