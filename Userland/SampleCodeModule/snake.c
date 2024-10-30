#include <snake.h>
#include <stdlib.h>
#include <shellfunctions.h>
#include <syscall.h>

#define MAX_PLAYERS 2
#define MAX_SNAKE_LENGTH 100
#define MAX_FOOD 100
#define FOOD_COLOR 0xffb600

enum gameOver {CONTINUE, PLAYER1_WIN, PLAYER2_WIN, GAME_DRAW};

#define SIZE 5 // 5pixel x 5pixel

typedef enum direction{
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction;

/////
typedef enum cellType{
    EMPTY,
    SNAKE1,
    SNAKE2,
    FOOD
} cellType;
/////

typedef struct Point2D{
    uint64_t x, y;
} Point2D;

typedef struct Snake{
    Point2D body[MAX_SNAKE_LENGTH];
    uint64_t length;
    uint64_t points;
    uint32_t color;
    direction dir;
} Snake;


uint64_t speed;
int gameOver = 0;
uint64_t width, height;
uint64_t speeds[] = {18, 15, 12, 9, 6, 3}; // 1 a 5. 
Point2D foodPosition;
char canGenerateFood = 1;
int cant_players = 1;


void runSnake(uint64_t speedLevel){ 
    
    if(speedLevel > 5){
        printf("Velocidad invalida. Los valores validos son [1,5]\n"); 
        return;
    } else {
        speed = speeds[speedLevel - 1];
    }
    start_screen();
    
    char buff[2];
    
    do{
        if(cant_players < 1 || cant_players > MAX_PLAYERS){
            printferror("-- Cantidad de jugadores invalida.\nLos valores validos son 1 o 2. Ingrese nuevamente ", cant_players);
        }
        readLine(buff, 2);
        cant_players = buff[0] - '0'; // ascii a int  
        printf("%d ", cant_players);
    } while (cant_players < 1 || cant_players > MAX_PLAYERS);
    char* text_player_1 = "\n\nJugador 1:\nPara moverse use las teclas W (UP), A (LEFT), S (DOWN), D (RIGHT)\n";
    char* text_player_2 = "Jugador 2:\nPara moverse use las teclas I (UP), J (LEFT), K (DOWN), L (RIGHT)\n";
    printf(text_player_1);
    if(cant_players == 2){
        printf("\n%s",text_player_2);
    }
    printf("\n\nPresione ENTER para comenzar o \"x\" para salir\n");
    char c;
    while((c = getChar()) != '\n' && c != 'x'){
        if(c == 'x'){
            return;
        }
    }
    clear();
    height = syscall(10,0,0,0); // getHeight();
    width = syscall(11,0,0,0); // getWidth();
    play(cant_players);
}   


void get(Snake snake1 , Snake snake2){
    char c;
    while((c = getChar()) == 0);
    switch(c){
        case 'w':
            moveSnake(snake1, UP);
            break;
        case 's':
            moveSnake(snake1, DOWN);
            break;
        case 'a':
            moveSnake(snake1, LEFT);
            break;
        case 'd':
            moveSnake(snake1, RIGHT);
            break;
        default:
            break;
    }
    if(cant_players == 2){
        switch(c){
            case 'i':
                moveSnake(snake2, UP);
                break;
            case 'k':
                moveSnake(snake2, DOWN);
                break;
            case 'j':
                moveSnake(snake2, LEFT);
                break;
            case 'l':
                moveSnake(snake2, RIGHT);
                break;
            default:
                break;
        }
    }
}


void play(int players){
    Snake snake1, snake2;
    snake1.points = 0;
    snake2.points = 0;
    setup(snake1, snake2);
    uint64_t counter = 1;
    
    while(gameOver == CONTINUE){
        if(counter % 3 == 0 && canGenerateFood){
            //generateFood();
            canGenerateFood = 0; // cuando come una se pone en 1
        }
        drawSnake(snake1);
        if(players == 2){
            drawSnake(snake2);
        }
        drawFood();


        get(snake1, snake2);
        
        checkSnakes(snake1, snake2, players);
        
        syscall(12,speed,0,0); // wait(ticks);
        counter++;
    }
    clear();
    syscall(8,2,0,0); // fontSizeUp(2);
    printf("GAME OVER\n");
    syscall(9,2,0,0); // fontSizeDown(2);
    
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
    default:
        break;
    }
    syscall(8,1,0,0); // fontSizeUp(1);
    printf("PUNTOS\n");
    syscall(9,1,0,0); // fontSizeDown(1);
    printf("Puntos Jugador 1: %d\n", snake1.points);
    if(players == 2){
        printf("Puntos Jugador 2: %d\n", snake2.points);
    }
    
}



void setup(Snake s1, Snake s2){ // dibuja los bordes
    Point2D upLeft, bottomRight;
    uint32_t wallColor = 0x7c7c7c;
    for(int i = 0; i < width ; i++){
        upLeft.x = i;
        upLeft.y = 0;
        bottomRight.x = i + 1;
        bottomRight.y = 1;
        syscall(5, &upLeft, &bottomRight, wallColor);
        upLeft.y = height - 2;
        bottomRight.y = height - 1;
        syscall(5, &upLeft, &bottomRight, wallColor);
    }
    for(int i = 0; i < height ; i++){
        upLeft.x = 0;
        upLeft.y = i;
        bottomRight.x = 1;
        bottomRight.y = i + 1;
        syscall(5, &upLeft, &bottomRight, wallColor);
        upLeft.x = width - 2;
        bottomRight.x = width - 1;
        syscall(5, &upLeft, &bottomRight, wallColor);
    }
    Point2D startSnake1 = {(width/2) - SIZE, height/2};
    Point2D startSnake2 = {(width/2) + SIZE, height/2};

    s1.body[0] = startSnake1;
    s2.body[0] = startSnake2;
}

void drawSnake(Snake snake) {
    Point2D upLeft = {1,1}, bottomRight = {width-2, height-2};
    
    syscall(5,&upLeft, &bottomRight, 0); // clear rectangle
    for(int i=0; i< snake.length; i++){
        bottomRight.x = snake.body[i].x + SIZE;
        bottomRight.y = snake.body[i].y + SIZE;
        syscall(5, &snake.body[i], &bottomRight, snake.color);
    }
}

void drawFood(){
    Point2D foodBottomRight;
    foodBottomRight.x = foodPosition.x + SIZE; 
    foodBottomRight.y = foodPosition.y + SIZE; 
    syscall(5, &foodPosition, &foodBottomRight , FOOD_COLOR);
}

void moveSnake(Snake snake, direction dir){
    for(int i = snake.length - 1; i > 0; i--){
        snake.body[i] = snake.body[i-1];
    }
    switch (dir){
        case UP:
            snake.body[0].y -= SIZE;
            break;
        case DOWN:
            snake.body[0].y += SIZE;
            break;
        case LEFT:
            snake.body[0].x -= SIZE;
            break;
        case RIGHT:
            snake.body[0].x += SIZE;
            break;
    }
}

void checkSnakes(Snake snake1, Snake snake2, int players){ 
    gameOver = checkSnakeBounds(snake1) ? PLAYER2_WIN : 0;
    if(players == 2){
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
        if(snake.body[i].x + SIZE > width || snake.body[i].x < 0  // CHEQUEAR LOS LIMITES
        || snake.body[i].y + SIZE > height || snake.body[i].y < 0){
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
    syscall(8,2,0,0); // fontSizeUp(2);
    printf("SNAKE\n");
    syscall(9,2,0,0); // fontSizeDown(2);
    printf("    Ingrese cantidad de jugadores: "); // provisorio. ver más adelante si se puede imprimir en el medio
}