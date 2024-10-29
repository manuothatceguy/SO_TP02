#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/**
 * @brief Realiza una interrupción 0x80 (SYSCALL)
 * @param code código de syscall
 * @param param... lo que corresponda para cada uno de los parámetros de la syscall. 0 si no se usa.
 * @return lo que devuelva la syscall
 */
extern int64_t syscall(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3);

#endif