#include <stdint.h>

//src : https://github.com/alejoaquili/ITBA-72.11-SO/tree/main/kernel-development/tests

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
void *my_memset(void *dest, int value, uint32_t count); //hecha a pelo para no usar externa
uint8_t memcheck(void *start, uint8_t value, uint32_t size);
int64_t satoi(char *str);
void bussy_wait(uint64_t n);
void endless_loop();
void endless_loop_print(uint64_t wait);
