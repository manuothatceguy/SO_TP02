#include <snake.h>
#include <stdlib.h>
#include <shellfunctions.h>
#include <syscall.h>
#include <snakeSound.h>

#define MAX_SPEED 5
#define MAX_PLAYERS 2
#define MAX_SNAKE_LENGTH 100
#define MAX_FOOD 100

#define FOOD_COLOR 0xffb600
#define SNAKE1_COLOR 0x00ff00
#define SNAKE2_COLOR 0xff0000
#define BLACK_COLOR 0x000000

#define SIZE 25 // Tamaño de la serpiente y la comida (pixeles)

#define SCALE_BY_SIZE(num) (((num) / SIZE) * SIZE)
#define CALCULATE_POINTS(snake) (((snake).length * SIZE * SIZE) / speed)

enum gameOver {CONTINUE , PLAYER1_WIN ,PLAYER2_WIN, GAME_DRAW, PLAYER1_LOSE}; // El ultimo para caso de un solo jugador

typedef enum direction{
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction;

typedef struct Snake{
    Point2D body[MAX_SNAKE_LENGTH];
    uint64_t length;
    uint32_t color;
    direction dir;
    uint64_t addTail; // flag para agregar cola SI COMIÓ
} Snake;

uint64_t speed;
int gameOver;
uint64_t max_Y, max_X, min_X, min_Y, realWidth, realHeight, snakeHeight, snakeWidth;
uint64_t speeds[] = {18*6, 15*6, 12*6, 9*6, 6*6, 3*6}; // 1 a 5.
Point2D foodPosition;
int cant_players;

void runSnake();
void get(Snake * snake1, Snake * snake2);
void play(int players);
uint64_t checkFood(Snake * s);
void generateFood(Snake s1, Snake s2);
void setup(Snake * s1, Snake * s2);
void drawSnake(Snake snake);
void drawFood();
void moveSnake(Snake * snake);

void clearSnake(Snake snake);
void drawColorSnake(Snake snake);

void checkSnakes(Snake snake1, Snake snake2);
int checkSnakeBounds(Snake snake);
int checkSnakeCollision(Snake mainSnake, Snake targetSnake);
void start_screen();

void runSnake(){ 
    syscall_clearScreen();
    start_screen();

    char buff[2];
    char aux;
    
    printf("  Ingrese cantidad de jugadores: "); // provisorio. ver más adelante si se puede imprimir en el medio
    do{
        readLine(buff, 2);
        cant_players = buff[0] - '0'; // ascii a int  
        if(cant_players < 1 || cant_players > MAX_PLAYERS){
            printferror("-- Cantidad de jugadores invalida.\nLos valores validos son 1 o 2. Ingrese nuevamente \n");
        }
    } while (cant_players < 1 || cant_players > MAX_PLAYERS);

    printf("  Ingrese velocidad: ");
    do{
        readLine(buff, 2);
        aux = buff[0] - '0';
        if(aux < 1 || aux > MAX_SPEED ){
            printferror("-- Velocidad invalida.\nEl rango de valores validos es [1-5]. Ingrese nuevamente: ");
        } else {
            speed = speeds[aux - 1];
        }
    } while(aux < 1 || aux > MAX_SPEED );

    char* text_player_1 = "\n\nJugador 1:\nPara moverse use las teclas W (UP), A (LEFT), S (DOWN), D (RIGHT)\n";
    char* text_player_2 = "Jugador 2:\nPara moverse use las teclas I (UP), J (LEFT), K (DOWN), L (RIGHT)\n";
    printf(text_player_1);
    if(cant_players == 2){
        printf("\n%s",text_player_2);
    }
    printf("\n\nPresione ENTER para comenzar, otro para salir\n");
    char c;
    while((c = getChar()) != '\n'){
        if(c != 0){
            syscall_clearScreen();
            return;
        }
    }
    syscall_clearScreen();

    realHeight = syscall_getHeight(); // getHeight();
    realWidth = syscall_getWidth(); // getWidth();

    snakeHeight = SCALE_BY_SIZE(realHeight - (SIZE * 2));
    snakeWidth = SCALE_BY_SIZE(realWidth - (SIZE * 2));
    
    min_X = (realWidth - snakeWidth) / 2;
    min_Y = (realHeight - snakeHeight) / 2;
    max_X = min_X + snakeWidth;
    max_Y = min_Y + snakeHeight;
    play(cant_players);
}   

void get(Snake * snake1 , Snake * snake2){
    char c = getChar();
    switch(c){
        case 'w':
            if(snake1->dir != DOWN){
                snake1->dir = UP;
            }
            break;
        case 's':
            if(snake1->dir != UP){
                snake1->dir = DOWN;
            }
            break;
        case 'a':
            if(snake1->dir != RIGHT){
                snake1->dir = LEFT;
            }
            break;
        case 'd':
            if(snake1->dir != LEFT){
                snake1->dir = RIGHT;
            }
            break;
        default:
            break;
    }
    if(cant_players == 2){
        switch(c){
            case 'i':
                if(snake2->dir != DOWN){
                snake2->dir = UP;
                }
                break;
            case 'k':
                if(snake2->dir != UP){
                snake2->dir = DOWN;
                }
                break;
            case 'j':
                if(snake2->dir != RIGHT){
                snake2->dir = LEFT;
                }
                break;
            case 'l':
                if(snake2->dir != LEFT){
                snake2->dir = RIGHT;
                }
                break;
            default:
                break;
        }
    }
}


void play(int players){
    gameOver = CONTINUE;
    Snake snake1, snake2;
    uint64_t appleEaten = 0;
    setup(&snake1, &snake2);

    generateFood(snake1, snake2);
    drawFood();

    while(gameOver == CONTINUE){

        clearSnake(snake1);
        if(players == 2){
            clearSnake(snake2);
        }

        get(&snake1, &snake2);
        moveSnake(&snake1);
        if(players == 2){
            moveSnake(&snake2);
        }

        drawSnake(snake1);
        if(players == 2){
            drawSnake(snake2);
        }

        checkSnakes(snake1, snake2);

        appleEaten += checkFood(&snake1); 
        if(cant_players == 2){
            appleEaten += checkFood(&snake2);
        }
        
        if( appleEaten ){
            // Sonido de comer una manzanita
            eatSound();
            generateFood(snake1, snake2); // cambio la comida de lugar
            drawFood();
            appleEaten = 0;
        }

        syscall_wait(speed); // wait(ticks);
    }
    deathSound(); // Ver si esta bien llamar a la funcion aca 
    syscall_clearScreen();
    syscall_sizeUpFont(2); // fontSizeUp(2);
    printf("GAME OVER\n\n");
    syscall_sizeDownFont(2); // fontSizeDown(2);
    
    switch (gameOver)
    {
    case PLAYER1_WIN:
        printf("Gana Jugador 1\n");
        break;
    case PLAYER2_WIN:
        printf("Gana Jugador 2\n");
        break;
    case GAME_DRAW:
        printf("Empate\n");
        break;
    case PLAYER1_LOSE:
        printf("Perdiste !\n");
        break;
    default:
        break;
    }
    syscall_sizeUpFont(1); // fontSizeUp(1);
    printf("\nPUNTOS\n\n");
    syscall_sizeDownFont(1); // fontSizeDown(1);
    printf("Puntos Jugador 1: %d\n", CALCULATE_POINTS(snake1));
    if(players == 2){
        printf("Puntos Jugador 2: %d\n", CALCULATE_POINTS(snake2));
    }
    while(getChar() != 0); // Consumir buffer
    printf("\n\nPresione ENTER para volver a la shell\n");
    while(getChar() != '\n');
    syscall_clearScreen();
}

int checkPoint(Snake s, Point2D p){
    // Booleano para ver si un punto esta dentro de una snake
    for(int i = 0 ; i < s.length ; i++){
        if(s.body[i].x == p.x && s.body[i].y == p.y){
            return 1;
        }
    }
    return 0;
}

void generateFood(Snake s1, Snake s2){
    int generated = 0; 
    while(!generated){
        
        int x = SCALE_BY_SIZE((randInt() % (snakeWidth))) + min_X;
        int y = SCALE_BY_SIZE((randInt() % (snakeHeight))) + min_Y;
        
        Point2D point;
        point.x = x;
        point.y = y;
        
        // CHEQUEO QUE NO COINCIDA CON NINGUNA DE LAS SERPIENTES
        int canGenerate = 1;
        if(checkPoint(s1, point)){
            canGenerate = 0;
        }

        if( cant_players == 2){
            if(checkPoint(s2, point)){
                canGenerate = 0;
            }
        }

        if(canGenerate){
            foodPosition.x = x;
            foodPosition.y = y;
            generated = 1;
        }
      
    }
}

// Chequea si comio algo
uint64_t checkFood(Snake * s){
    if(s->body[0].x == foodPosition.x && s->body[0].y == foodPosition.y){
        s->addTail = 1;
        return 1;
    }
    return 0;
}

void setup(Snake * s1, Snake * s2){ // dibuja los bordes
    Point2D upLeft, bottomRight;
    uint32_t wallColor = 0x7c7c7c;
    for(int i = min_X; i < max_X ; i++){
        upLeft.x = i;
        upLeft.y = min_Y-4;
        bottomRight.x = i + 1;
        bottomRight.y = min_Y-1;
        syscall_drawRectangle(&upLeft, &bottomRight, wallColor);
        upLeft.y = max_Y+1;
        bottomRight.y = max_Y+4;
        syscall_drawRectangle(&upLeft, &bottomRight, wallColor);
    }
    for(int i = min_Y-1; i < max_Y+1 ; i++){ // -1 y +1 para incluir vertices
        upLeft.x = min_X-4;
        upLeft.y = i;
        bottomRight.x = min_X-1;
        bottomRight.y = i + 1;
        syscall_drawRectangle(&upLeft, &bottomRight, wallColor);
        upLeft.x = max_X+1;
        bottomRight.x = max_X+4;
        syscall_drawRectangle(&upLeft, &bottomRight, wallColor);
    }

    Point2D startSnake1 = {SCALE_BY_SIZE(snakeWidth/2) + min_X - SIZE, SCALE_BY_SIZE(snakeHeight/2) + min_Y}; 
    Point2D startSnake2 = {SCALE_BY_SIZE(snakeWidth/2) + min_X + SIZE, SCALE_BY_SIZE(snakeHeight/2) + min_Y}; 

    s1->body[0] = startSnake1;
    s1->length = 1;
    s1->color = SNAKE1_COLOR;
    s1->dir = UP; 
    s1->addTail = 0;

    if(cant_players == 2){
        s2->body[0] = startSnake2;
        s2->length = 1;
        s2->color = SNAKE2_COLOR;
        s2->dir = DOWN;
        s2->addTail = 0;
    }
}

void drawSnake(Snake snake){
    drawColorSnake(snake);
}

void clearSnake(Snake snake){
    snake.color = BLACK_COLOR; 
    drawColorSnake(snake); // Aca no estoy cambiando el color original de la snake porque me pasan una copia!!!!
}

void drawColorSnake(Snake snake) {
    Point2D bottomRight;

    for(int i=0; i< snake.length; i++){
        bottomRight.x = snake.body[i].x + SIZE;
        bottomRight.y = snake.body[i].y + SIZE;
        syscall_drawRectangle(&snake.body[i], &bottomRight, snake.color);
    }
}

char apple[SIZE][SIZE] = {
    "0000000000000011110000000",
    "0000000000000110010000000",
    "0000000000001100000000000",
    "0000000111111111111000000",
    "0000011111111111111110000",
    "0001111111111111111111100",
    "0011111111111111111111110",
    "0111111111111111111111111",
    "0111111111111111111111111",
    "1111111111111111111111111",
    "1111111111111111111111111",
    "1111111111111111111111111",
    "1111111111111111111111111",
    "1111111111111111111111111",
    "0111111111111111111111111",
    "0011111111111111111111111",
    "0011111111111111111111111",
    "0001111111111111111111110",
    "0001111111111111111111110",
    "0000111111111111111111100",
    "0000011111111111111111000",
    "0000001111111111111110000",
    "0000000111111111111100000",
    "0000000011111111110000000",
    "0000000000011111000000000"
};


void drawFood(){ 
    //Point2D foodBottomRight;
    //foodBottomRight.x = foodPosition.x + SIZE; 
    //foodBottomRight.y = foodPosition.y + SIZE; 
    //syscall(5, (uint64_t)&foodPosition, (uint64_t)&foodBottomRight , FOOD_COLOR);
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++){
            syscall_drawRectangle(&(Point2D){foodPosition.x + j, foodPosition.y + i}, &(Point2D){foodPosition.x + j + 1, foodPosition.y + i + 1}, (apple[i][j] == '1') ? FOOD_COLOR : BLACK_COLOR);
        }
    }
}

void moveSnake(Snake * snake){
    Point2D tail = snake->body[snake->length - 1];

    for(int i = snake->length - 1; i > 0; i--){
        snake->body[i] = snake->body[i-1];
    }
    switch (snake->dir){
        case UP:
            snake->body[0].y -= SIZE;
            break;
        case DOWN:
            snake->body[0].y += SIZE;
            break;
        case LEFT:
            snake->body[0].x -= SIZE;
            break;
        case RIGHT:
            snake->body[0].x += SIZE;
            break;
    }

    if(snake->addTail){
        snake->addTail = 0;
        snake->length++;
        snake->body[snake->length - 1] = tail;
    }
}

void checkSnakes(Snake snake1, Snake snake2){ 
    gameOver = checkSnakeBounds(snake1) ? PLAYER1_LOSE : 0;  // vER SI SE ARREGLO 
    if(cant_players == 2){
        gameOver = checkSnakeBounds(snake2) ? PLAYER1_WIN : 0;
        int collision1 = checkSnakeCollision(snake1, snake2);
        int collision2 = checkSnakeCollision(snake2, snake1);

        if(collision1 && collision2) {
            gameOver = GAME_DRAW; // Ambos colisionan --> empate
        } else if(collision1) {
            gameOver = PLAYER2_WIN; // Colisiona 1 con 2 --> gana 2
        } else if(collision2) {
            gameOver = PLAYER1_WIN; // Colisiona 2 con 1 --> gana 1
        }
    }
}

int checkSnakeBounds(Snake snake){
    for(int i=0; i<snake.length; i++){
        if(snake.body[i].x + SIZE > max_X || snake.body[i].x < min_X  // CHEQUEAR LOS LIMITES, ACA NO HABRIA QUE PONER >= PARA QUE NO SE VAYA DE LA PANTALLA EL ULTIMO SEGUNDO ? @goyo
        || snake.body[i].y + SIZE > max_Y || snake.body[i].y < min_Y){
            return 1;
        } 
    }
    for(int i=1; i<snake.length; i++){
        if(snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y){
            return 1; 
        }
    }
    return 0;
}

int checkSnakeCollision(Snake mainSnake, Snake targetSnake){
    for(int i=0; i<targetSnake.length; i++){
        if(mainSnake.body[0].x == targetSnake.body[i].x && mainSnake.body[0].y == targetSnake.body[i].y ){
            return 1; // ¡GANARIA targetSnake!
        }
    }
    return 0;
}

void start_screen(){
    syscall_sizeUpFont(2); // fontSizeUp(2);
    printf("SNAKE\n");
    syscall_sizeDownFont(2); // fontSizeDown(2);
}