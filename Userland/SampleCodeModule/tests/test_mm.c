// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <syscall.h>
#include "test_util.h"
#include <stdlib.h>
#include <test_functions.h>

//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

#define MAX_BLOCKS 128

typedef struct MM_rq {
  void *address;
  uint32_t size;
} mm_rq;

uint64_t test_mm(uint64_t argc, char *argv[]) {
  mm_rq mm_rqs[MAX_BLOCKS];
  //printf("test_mm: max_memory = %s\n", argv[0]);
  uint8_t rq;
  uint32_t total;
  uint64_t max_memory;

  if (argc != 1){
    printferror("test_mm: error, se esperaba un argumento (max_memory)\n");
    return -1;
  }
  max_memory = satoi(argv[0]);
  printf("max_memory: %d\n", max_memory);
  if (max_memory <= 0){
    printferror("\ntest_mm: error, max_memory debe ser un numero positivo\n");
    return -1;
  }

  //char curr = 'a';
  //printf("antes del while 1");
  char j = 0;
  for (; j < 3; j++) {
    rq = 0;
    total = 0;
    //printf("antes del otro while\n");
    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      //printf("%c\n", curr);
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      mm_rqs[rq].address = malloc(mm_rqs[rq].size);

      if (mm_rqs[rq].address) {
        total += mm_rqs[rq].size;
        rq++;
      }
      //curr++;
      //curr = curr % 26 + 'a';
    }

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address)
        my_memset(mm_rqs[i].address, i, mm_rqs[i].size);

    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          printferror("\ntest_mm ERROR\n");
        }

    // Free
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address)
        free(mm_rqs[i].address);
  }
  printf("\ntest_mm: finished successfully\n");
  return 0;
}
