#ifndef SHELLFUNCTIONS_H
#define SHELLFUNCTIONS_H

#include <stdarg.h>
#include <stdint.h>

void showTime();
void showRegisters();

void handle_help(char *arg);
void handle_time(char *arg);
void handle_registers(char *arg);
void handle_echo(char *arg);
void handle_size_up(char *arg);
void handle_size_down(char *arg);
void handle_test_div_0(char *arg);
void handle_test_invalid_opcode(char *arg);
void handle_clear(char *arg);
void handle_test_mm(char * arg);
void handle_test_prio(char * arg);
void handle_test_processes(char * arg);
void handle_test_sync(char * arg);
void handle_mem_status(char * arg);


#endif //SHELLFUNCTIONS_H
