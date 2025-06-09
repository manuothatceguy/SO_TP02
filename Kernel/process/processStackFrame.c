// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <defs.h>
#include <memoryManager.h>
#include <processStackFrame.h>
#include <lib.h>
#include <process.h>

static char** copyArgsToProcessStack(uint64_t* paramsBase, uint64_t argc, char **argv);

uint64_t setupProcessStack(uint64_t* stackBase, uint64_t entryPoint, uint64_t argc, char **argv) {
    *stackBase = (uint64_t)allocMemory(STACK_SIZE);
    if(*stackBase == 0){ // NULL
        return 0;
    }
    *stackBase += STACK_SIZE; 
    uint64_t paramsBase = *stackBase;
    char** newArgv = copyArgsToProcessStack(&paramsBase, argc, argv);
    uint64_t finalRSP = processStackFrame(paramsBase, entryPoint, argc, newArgv);
    return finalRSP;
}

static char** copyArgsToProcessStack(uint64_t* paramsBase, uint64_t argc, char **argv){
    char ** newArgv = NULL;
    if(argc > 0 && argv != NULL){
        // Reservar espacio para array de punteros argv
        *paramsBase -= sizeof(char*) * (argc + 1);  // +1 para NULL terminator
        newArgv = (char**)*paramsBase;
        
        // Copiar cada string al stack
        for (int i = argc - 1; i >= 0; i--) {  // Iterar hacia atrás
            int len = strlen(argv[i]) + 1;
            *paramsBase -= len;                      // Reservar espacio para string
            strncpy((char*)*paramsBase, argv[i], len);   // Copiar string
            newArgv[i] = (char*)*paramsBase;         // Apuntar al string copiado
        }
        newArgv[argc] = NULL;  // NULL terminator
    }
    return newArgv; 
}
