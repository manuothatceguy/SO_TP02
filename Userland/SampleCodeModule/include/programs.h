#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>
#include <stdlib.h>
#include <shell.h>
#include "../../../SharedLibraries/shared_structs.h"

#define CANT_REGISTERS 19

//utils
int parse_string(char *arg, char **args, int max_args, int max_size);

//shellfunctions
void printHeader();
void printProcessInfo(PCB processInfo); 

//programs
void loop(uint64_t argc, char *argv[]);
uint64_t cat(uint64_t argc, char *argv[]);
uint64_t wc(uint64_t argc, char *argv[]);
uint64_t filter(uint64_t argc, char *argv[]);

#endif