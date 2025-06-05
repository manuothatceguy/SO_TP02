#include <syscall.h>
#include "test_util.h"
#include <test_functions.h>

//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

//enum State { RUNNING,
//             BLOCKED,
//             KILLED };

//typedef struct P_rq {
//  int32_t pid;
//  enum State state;
//} p_rq;

typedef struct P_rq {
  int32_t pid;
  ProcessState state;
} p_rq;

int64_t test_processes(uint64_t argc, char *argv[]) {
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  char *argvAux[] = {0};

  if (argc != 1){
    printf("test_processes: ERROR: argc != 1\n");
    return -1;
  }
    

  if ((max_processes = satoi(argv[0])) <= 0){
    printf("test_processes: ERROR: max_processes <= 0\n");
    return -1;
  }
  p_rq p_rqs[max_processes];

  for (int i = 0; i < 10; i++) {
    printf("i: %d\n", i);
    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++) {
      p_rqs[rq].pid = syscall_create_process("endless_loop",(fnptr) endless_loop, 0, argvAux, 2, 0);

      if (p_rqs[rq].pid == -1) {
        printf("test_processes: ERROR creating process\n");
        return -1;
      } else {
        p_rqs[rq].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {
      printf("alive: %d\n", alive);
      for (rq = 0; rq < max_processes; rq++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
              if (syscall_kill(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR killing process\n");
                return -1;
              }
              //p_rqs[rq].state = KILLED;
              p_rqs[rq].state = EXITED;
              alive--;
            }
            break;

          case 1:
            if (p_rqs[rq].state == RUNNING) {
              if (syscall_block(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR blocking process\n");
                return -1;
              }
              p_rqs[rq].state = BLOCKED;
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2) {
          if (syscall_unblock(p_rqs[rq].pid) == -1) {
            printf("test_processes: ERROR unblocking process\n");
            return -1;
          }
          p_rqs[rq].state = RUNNING;
        }
    }
  }
  printf("si llegue aca esta todo joya\n");
  return 0;
}
