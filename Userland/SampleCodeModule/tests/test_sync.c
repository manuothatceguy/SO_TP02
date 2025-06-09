// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <syscall.h>
#include <stdlib.h>
#include "test_util.h"
#include <test_functions.h>

//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

#define SEM_ID 150
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc) {
  uint64_t aux = *p;
  syscall_yield(); // This makes the race condition highly probable
  aux += inc;
  *p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]) {
  uint64_t n;
  int8_t inc;
  int8_t use_sem;

  if (argc != 3)
    return -1;

  if ((n = satoi(argv[0])) <= 0)
    return -1;
  if ((inc = satoi(argv[1])) == 0)
    return -1;
  if ((use_sem = satoi(argv[2])) < 0)
    return -1;

  if (use_sem)
    if (syscall_sem_open(SEM_ID, 1) == -1) {
      printf("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++) {
    if (use_sem) {
      syscall_sem_wait(SEM_ID);
    }
    slowInc(&global, inc);
    
    if (use_sem)
      syscall_sem_post(SEM_ID);
  }

  if (use_sem)
    syscall_sem_close(SEM_ID);

  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]) { //{n, use_sem, 0}
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2){
    printferror("test_sync: ERROR: expected 2 arguments, got %d\n", argc);
    return -1;
  }
    

  char *argvDec[] = {argv[0], "-1", argv[1], NULL};
  char *argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;

  uint64_t i;
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    pids[i] = syscall_create_process("my_process_inc", my_process_inc, argvDec, 1, 1, 0, 1);
    pids[i + TOTAL_PAIR_PROCESSES] = syscall_create_process("my_process_inc", my_process_inc, argvInc, 1, 1, 0, 1);
  }

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    syscall_waitpid(pids[i], NULL);
    syscall_waitpid(pids[i + TOTAL_PAIR_PROCESSES], NULL);
  }

  printf("Final value: %d\n", global);

  return 0;
}
