#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stdarg.h>
#include <syscall.h>

#define NULL (void*) 0

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
 * @brief Imprime en pantalla un string formateado de error
 * @param format string con formato
 * @param ... argumentos a imprimir
 * @return cantidad de caracteres impresos
 */
uint64_t printferror(const char *format, ...);

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
 * @param n entero a convertir
 * @param buff buffer donde se guardará el string
 */
void intToStr(int n, char * buff);

/**
 * @brief Convierte un entero a string en hexadecimal
 * @param n entero a convertir
 * @param buff buffer donde se guardará el string
 */
void intToHex(int n, char * buff);

/**
 * @brief Obtiene un carácter de la entrada estándar.
 * 
 * Esta función espera y devuelve un único carácter ingresado por el usuario.
 * No requiere parámetros y devuelve el carácter como tipo char.
 * 
 * @return El carácter ingresado por el usuario.
 */
char getChar();

/**
 * @brief Genera un número entero sin signo aleatorio.
 *
 * Esta función devuelve un valor entero sin signo aleatorio.
 *
 * @return Un número entero sin signo aleatorio.
 */
unsigned int randInt();

void * malloc(uint64_t size);

void free(void *ptr);

#endif // STDLIB_H