#include <snake.h>
#include <stdlib.h>
#include <syscall.h>

void runSnake(){ 
    start_screen();
    printf("SNAKE\n");
    printf("Ingrese cantidad de jugadores: "); // provisorio. ver más adelante si se puede imprimir en el medio
    char buff[2];
    if(readLine(buff,2) != 2 || (buff[0] < '1' || buff[0] > '2')){
        // error;
    }
    int cant_players = buff[0] - '0'; // ascii a int
    play(cant_players);
}   

void play(int players){

}

void start_screen(){
    // poner que diga snake y que ingrese cantidad de jugadores seguido de enter.
}