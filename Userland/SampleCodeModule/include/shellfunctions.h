#ifndef SHELL_FUNCTIONS_H
#define SHELL_FUNCTIONS_H

#include <stdarg.h>
#include <stdint.h>
#include <test_functions.h>

typedef struct command{
    int instruction;
    char * arguments;
} command;

typedef struct pipecmd{
    command cmd1;
    command cmd2;
} pipeCmd;




void showTime();
void showRegisters();

void handle_help(char * arg, int sdtin, int stdout);
void handle_time(char * arg, int sdtin, int stdout);
void handle_registers(char * arg, int sdtin, int stdout);
void handle_echo(char * arg, int sdtin, int stdout);
void handle_size_up(char * arg, int sdtin, int stdout);
void handle_size_down(char * arg, int sdtin, int stdout);
void handle_test_div_0(char * arg, int sdtin, int stdout);
void handle_test_invalid_opcode(char * arg, int sdtin, int stdout);
void handle_clear(char * arg, int sdtin, int stdout);
void handle_test_mm(char * arg, int sdtin, int stdout);
void handle_test_processes(char * arg, int sdtin, int stdout);
void handle_test_prio(char * arg, int sdtin, int stdout);
void handle_test_sync(char * arg, int sdtin, int stdout);
void handle_ps(char * arg, int sdtin, int stdout);
void handle_mem_info(char * arg, int sdtin, int stdout);
void handle_loop(char * arg, int sdtin, int stdout);
void handle_pid_info(char * arg, int sdtin, int stdout);
void handle_nice(char * arg, int sdtin, int stdout);
void handle_wc(char * arg, int sdtin, int stdout);
void handle_filter(char * arg, int sdtin, int stdout);
void handle_cat(char * arg, int sdtin, int stdout);
void handle_test_malloc_free(char * arg, int sdtin, int stdout);
void handle_phylo(char * arg, int sdtin, int stdout);

#endif //SHELL_FUNCTIONS_H
