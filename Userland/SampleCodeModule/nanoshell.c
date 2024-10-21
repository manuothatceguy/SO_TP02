#include <nanoshell.h>

char * help = "Para ayuda relacionada a un comando en especifico, ingrese el comando \"help\" seguido del comando.\n"
                "Lista de comandos disponibles en la Shell:\n"
                "help                   Muestra lista de comandos disponibles en la Shell\n"
                "snake                  Inicia un juego de Snake\n"
                "time                   Muestra en pantalla el horario actual\n"
                "registers              Imprime el valor de los registros en tiempo real\n"
                "test_div_0             Prueba la excepcion que salta al intentar dividir por 0\n"
                "test_invalid_opcode    Prueba la excepcion que salta al ingresar un codigo de operacion invalido\n";

typedef enum {
    HELP = 1,
    SNAKE,
    TIME,
    REGISTERS,
    TEST_DIV_0,
    TEST_INVALID_OPCODE
} instructions;

/*
Ver lo de la funcion readline para switch(instruction)
*/

void nanoshell() {
    // Ver como hacer getInstructions
    Instructions instruction; // Assuming you will get the instruction somehow
    switch(instruction) {
        case HELP: {
            printf(help); // Usando nuestro printf
            break;
        }
        case SNAKE: {
            runSnake(); // Ver que parametros recibe
            break;
        }
        case TIME: {
            showTime();
            break;
        }
        case REGISTERS: {
            showRegisters();
            break;
        }
        case TEST_DIV_0: {
            divZero();
            break;
        }
        case TEST_INVALID_OPCODE: {
            invalidOpcode();
            break;
        }
        default : {
            invalidOpcode();
        }
    }
}
