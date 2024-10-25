#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stdarg.h>

/**
 * @brief Longitud de un string null-terminated
 * @param s string para sacarle la longitud
 *
 */
uint64_t strlen(char * s);

/**
 * @brief Imprime en pantalla un string formateado
 * @param format string con formato
 * @param ... argumentos a imprimir
 * @return cantidad de caracteres impresos
 */
uint64_t printf(const char *format, ...);

/**
 * @brief Lee de la entrada estandar un string
 * @param buff Buffer donde poner lo leído
 * @param length cantidad de caracteres a leer (menor o igual al tamaño del buffer)
 * @return cantidad de caracteres leidos
 */
uint64_t readLine(char * buff, uint64_t length);

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