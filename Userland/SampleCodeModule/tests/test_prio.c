// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <syscall.h>
#include "test_util.h"
#include <test_functions.h>
#include <stdlib.h>

//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

#define MINOR_WAIT 100000000 // TODO: Change this value to prevent a process from flooding the screen
#define WAIT 100000000000      // TODO: Change this value to make the wait long enough to see theese processes beeing run at least twice

#define TOTAL_PROCESSES 3
#define LOWEST 5  // TODO: Change as required
#define MEDIUM 3  // TODO: Change as required
#define HIGHEST 0 // TODO: Change as required

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};


void print_a(){
  while (1) {
    printf("A");
    bussy_wait(MINOR_WAIT);
  }
}

void print_b(){
  while (1) {
    printf("B");
    bussy_wait(MINOR_WAIT);
  }
}

void print_c(){
  while (1) {
    printf("C");
    bussy_wait(MINOR_WAIT);
  }
}
//uint64_t syscall_create_process(char *name, fnptr function, uint64_t argc, char *argv[], uint8_t priority);

#define LOOPS 10

uint64_t test_prio() {
  printf("TEST DE PRIORIDADES\n");
  // printf("Creando %d procesos con diferentes prioridades\n", TOTAL_PROCESSES);
  
  int64_t pids[TOTAL_PROCESSES];
  char *argv[] = {0};

  // printf("Descripcion del test:\n");
  // printf("Se crean %d procesos que imprimen letras A, B y C respectivamente.\n", TOTAL_PROCESSES);
  // printf("Cada proceso tiene una prioridad diferente:\n");
  // printf(" - Proceso A: Prioridad %d (LOWEST)\n", LOWEST);
  // printf(" - Proceso B: Prioridad %d (MEDIUM)\n", MEDIUM);
  // printf(" - Proceso C: Prioridad %d (HIGHEST)\n", HIGHEST);
  // printf("Los procesos A y B deberian imprimir sus letras con menor frecuencia que el proceso C.\n");

  // bussy_wait(WAIT);

  // printf("Creando los procesos\n");

  // pids[0] = syscall_create_process("print_a",(fnptr)print_a, 0, argv, LOWEST, 1, 0, 1);
  // if( pids[0] < 0) {
  //   printferror("Error al crear el proceso A\n");
  //   return -1;
  // }
  // printf("Proceso A creado con PID: %d\n", (int)pids[0]);
  // pids[1] = syscall_create_process("print_b",(fnptr)print_b, 0, argv, MEDIUM, 1, 0, 1);
  // if( pids[1] < 0) {
  //   printferror("Error al crear el proceso B\n");
  //   return -1;
  // }
  // printf("Proceso B creado con PID: %d\n", (int)pids[1]);
  // pids[2] = syscall_create_process("print_c",(fnptr)print_c, 0, argv, HIGHEST, 1, 0, 1);
  // if( pids[2] < 0) {
  //   printferror("Error al crear el proceso C\n");
  //   return -1;
  // }
  // printf("Proceso C creado con PID: %d\n", (int)pids[2]);
  // printf("Los procesos deberian comenzar a imprimir sus letras en la pantalla.\n");
  
  uint64_t i;

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = syscall_create_process("endless_loop_print", (fnptr)endless_loop_print, argv, prio[i], 1, 0, 1);

  for (char i = 0; i < LOOPS; i++){
    syscall_yield();
  }
  
  printf("\nCHANGING PRIORITIES...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    syscall_changePrio((uint64_t)pids[i], (int8_t)prio[i]);

  bussy_wait(WAIT);
  printf("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    syscall_block((uint64_t)pids[i]);

  printf("CHANGING PRIORITIES WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    syscall_changePrio(pids[i], MEDIUM);

  printf("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    syscall_unblock((uint64_t)pids[i]);

  bussy_wait(WAIT);
  printf("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++){
    syscall_kill((uint64_t)pids[i]);
    syscall_waitpid((uint64_t)pids[i], NULL);
  }
    
  
  return 0;
}
