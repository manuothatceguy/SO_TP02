#ifndef SHELL_FUNCTIONS_H
#define SHELL_FUNCTIONS_H

#include <stdarg.h>
#include <stdint.h>
#include <test_functions.h>

void showTime();
void showRegisters();

void handle_help(char * arg);
void handle_time(char * arg);
void handle_registers(char * arg);
void handle_echo(char * arguments);
void handle_size_up(char * arg);
void handle_size_down(char * arg);
void handle_test_div_0(char * arg);
void handle_test_invalid_opcode(char * arg);
void handle_clear(char * arg);
void handle_test_mm(char * arg);
void handle_test_processes(char * arg);
void handle_test_prio(char * arg);
void handle_test_sync(char * arg);
void handle_ps(char * arg);
void handle_mem_info(char * arg);

#endif //SHELL_FUNCTIONS_H
