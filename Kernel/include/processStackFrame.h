#ifndef PROCESS_STACK_FRAME_H
#define PROCESS_STACK_FRAME_H

#include <shared_structs.h>

uint64_t setupProcessStack(uint64_t * stackBase, uint64_t entryPoint, uint64_t argc, char **argv);

// Assembly functions
uint64_t processStackFrame(uint64_t base, uint64_t entryPoint, uint64_t argc, char **arg);
void wrapper(uint64_t entryPoint, char **argv);

#endif