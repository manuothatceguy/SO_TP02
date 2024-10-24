#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>

/**
 * @brief Longitud de un string null-terminated
 * @param s string para sacarle la longitud
 *
 */
u_int64_t strlen(char * s);

/**
 * @brief Imprime en pantalla un string formateado
 * @param format string con formato
 * @param ... argumentos a imprimir
 * @return cantidad de caracteres impresos
 */
int printf(const char *format, ...);

/**
 * @brief Lee de la entrada estandar un string formateado
 * @param format string con formato
 * @param ... punteros a los argumentos a leer
 * @return cantidad de caracteres leidos
 */
int scanf(const char *format, ...);

/**
 * @brief Compara dos strings
 * @param s1 primer string
 * @param s2 segundo string
 * @return 0 si son iguales, otro valor si no
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Convierte un entero a string
 * @param n entero a imprimir
 * @return string con el entero
 */
char * intToStr(int n);

/**
 * @brief Convierte un entero a string en hexadecimal
 * @param n entero a imprimir
 * @return string con el entero en hexadecimal
 */
char * intToHex(int n);

#endif // STDLIB_H