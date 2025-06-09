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

pid_t handle_help(char * arg, int sdtin, int stdout);
pid_t handle_echo(char * arg, int sdtin, int stdout);
pid_t handle_clear(char * arg, int sdtin, int stdout);
pid_t handle_test_mm(char * arg, int sdtin, int stdout);
pid_t handle_test_processes(char * arg, int sdtin, int stdout);
pid_t handle_test_prio(char * arg, int sdtin, int stdout);
pid_t handle_test_sync(char * arg, int sdtin, int stdout);
pid_t handle_ps(char * arg, int sdtin, int stdout);
pid_t handle_mem_info(char * arg, int sdtin, int stdout);
pid_t handle_loop(char * arg, int sdtin, int stdout);
pid_t handle_pid_info(char * arg, int sdtin, int stdout);
pid_t handle_nice(char * arg, int sdtin, int stdout);
pid_t handle_wc(char * arg, int sdtin, int stdout);
pid_t handle_filter(char * arg, int sdtin, int stdout);
pid_t handle_cat(char * arg, int sdtin, int stdout);
pid_t handle_test_malloc_free(char * arg, int sdtin, int stdout);
pid_t handle_phylo(char * arg, int sdtin, int stdout);

#endif //SHELL_FUNCTIONS_H
