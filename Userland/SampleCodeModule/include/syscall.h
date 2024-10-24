#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stdarg.h>

/**
 * @brief Realiza una interrupción 0x80 (SYSCALL)
 * @param code código de syscall
 * @param count cantidad de argumentos a pasar
 * @return lo que devuelva la syscall
 */
uint64_t syscall(uint64_t code, int count,...);

#endif