/***************************************************
  Defs.h
****************************************************/

#ifndef _defs_
#define _defs_

#include <stdint.h>

/* Flags para derechos de acceso de los segmentos */
#define ACS_PRESENT     0x80            /* segmento presente en memoria */
#define ACS_CSEG        0x18            /* segmento de codigo */
#define ACS_DSEG        0x10            /* segmento de datos */
#define ACS_READ        0x02            /* segmento de lectura */
#define ACS_WRITE       0x02            /* segmento de escritura */
#define ACS_IDT         ACS_DSEG
#define ACS_INT_386 	  0x0E		/* Interrupt GATE 32 bits */
#define ACS_INT         ( ACS_PRESENT | ACS_INT_386 )

#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

#define MEMORY_MANAGER_ADDRESS 0x50000                  // aprovechando espacio vacío de la memoria
#define HEAP_START_ADDRESS 0x600000                     // dirección de inicio del heap
#define HEAP_SIZE (16 * 1024 * 1024) // 16MB
#define HEAP_END_ADDRESS   (HEAP_START_ADDRESS + HEAP_SIZE) // fin del heap

// Tamaño del stack del proceso en modo usuario (por ejemplo, 16 KiB)
#define USER_STACK_SIZE  0x4000      // 16 KiB

// Dirección virtual más alta del stack de usuario (debe estar fuera del espacio del kernel)
#define USER_STACK_TOP   0x800000    // 8 MB

//#define NULL (void *)0

#endif