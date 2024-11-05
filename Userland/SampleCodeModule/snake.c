#include <snake.h>
#include <stdlib.h>
#include <shellfunctions.h>
#include <syscall.h>
#include <snakeSound.h>

#define MAX_SPEED 5
#define MAX_PLAYERS 2
#define MAX_SNAKE_LENGTH 100 

#define FOOD_COLOR 0xffb600
#define SNAKE1_COLOR 0x00ff00
#define SNAKE2_COLOR 0xff0000
#define BLACK_COLOR 0x000000
#define WALL_COLOR 0x7c7c7c

#define SIZE 25 // Tamaño de la serpiente y la comida (pixeles)
#define TITLE_SIZE 15 // Tamaño de la fuente del titulo
#define TITLE_LEN 8

#define SCALE_BY_SIZE(num) (((num) / SIZE) * SIZE)
#define CALCULATE_POINTS(length) ((length * SIZE * SIZE) / speed)

enum gameOver {CONTINUE , PLAYER1_WIN ,PLAYER2_WIN, GAME_DRAW, PLAYER1_LOSE}; // El ultimo para caso de un solo jugador

typedef enum snakeID {
    SNAKE1,
    SNAKE2
} snakeID;

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
    uint64_t addTail;
} Snake;


void runSnake();
void get();
void play();

void setup();
void setupSnake(snakeID id, Point2D start, uint32_t color, direction dir);

void updateSnake();
void updateFood();

void drawScores();

void drawColorSnake(snakeID id, uint32_t color);

uint64_t checkFood(snakeID id);
void generateFood();
void drawFood();

void moveSnake(snakeID id);

void checkSnakes();
int checkSnakeBounds(snakeID id);
int checkSnakeCollision(Snake mainSnake, Snake targetSnake);

void start_screen();

Snake snakes[MAX_PLAYERS];
uint64_t appleEaten;

uint64_t speed;
int gameOver, generated, canGenerate;
uint64_t max_Y, max_X, min_X, min_Y, realWidth, realHeight, snakeHeight, snakeWidth;
uint64_t speeds[] = {18*6, 15*6, 12*6, 9*6, 6*6, 3*6}; // 1 a 5.
Point2D foodPosition;
int cant_players, lastScoreLen = 0;

char * snakeTitle[9] = {
" ",
"   222222222  222222   22222   222222222   22222   2222 2222222222",
"  22211111122 11222222 11222  222111111222 11222  22211 1122211112",
" 1222    111  11221122 11222 1222    11222  1122 222    112221  1 ",
" 11222222222  1122 112211222  222222222222  2222222     2222222   ",
"  11111111122 1122 112222222  222211111222  1122 11222  112221    ",
"  222    1122 1122  1122222  1222    11222  1122  11222 112221   2",
" 11222222222 222222  1122222 12222   22222 22222  222222 2222222222",
"  1111111111 111111    1111  11111   11111 11111   1111 1111111111 "
};

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

void runSnake(){ 
    syscall_clearScreen();
    start_screen();

    char buff[2];
    char aux;
    
    printf("  Ingrese cantidad de jugadores: ");
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

    char* text_player_1 = "\n\nJugador 1:\nPara moverse use las teclas \n   W (UP), A (LEFT), S (DOWN), D (RIGHT)\n";
    char* text_player_2 = "Jugador 2:\nPara moverse use las teclas \n   I (UP), J (LEFT), K (DOWN), L (RIGHT)\n";
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

    realHeight = syscall_getHeight();
    realWidth = syscall_getWidth();

    snakeHeight = SCALE_BY_SIZE(realHeight - (SIZE * 2));
    snakeWidth = SCALE_BY_SIZE(realWidth - (SIZE * 2));
    
    min_X = (realWidth - snakeWidth) / 2;
    min_Y = (realHeight - snakeHeight) / 2;
    max_X = min_X + snakeWidth;
    max_Y = min_Y + snakeHeight;
    play();
}   

void play(){
    lastScoreLen = 0;
    gameOver = CONTINUE;
    setup();

    generateFood();
    drawFood();
    printf("                         ");

    while(gameOver == CONTINUE){

        updateSnake();
        updateFood();
        drawScores();

        if(snakes[SNAKE1].length == snakeHeight * snakeWidth / SIZE){
            gameOver = PLAYER1_WIN;
        } else if(cant_players == 2 && snakes[SNAKE2].length == snakeHeight * snakeWidth / SIZE){
            gameOver = PLAYER2_WIN;
        }
        syscall_wait(speed); // wait(ticks);
    }
    deathSound(); 
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
    syscall_sizeUpFont(1);
    printf("\nPUNTOS\n\n");
    syscall_sizeDownFont(1);
    printf("Puntos Jugador 1: %d\n", CALCULATE_POINTS(snakes[SNAKE1].length));
    if(cant_players == 2){
        printf("Puntos Jugador 2: %d\n", CALCULATE_POINTS(snakes[SNAKE2].length));
    }
    while(getChar() != 0); // Consumir buffer
    printf("\n\nPresione ENTER para volver a la shell\n");
    while(getChar() != '\n');
    syscall_clearScreen();
}

void updateSnake(){
    for(int i=0; i<cant_players; i++)
        drawColorSnake(i, BLACK_COLOR);
    get();
    for(int i=0; i<cant_players; i++)
        moveSnake(i);
    for(int i=0; i<cant_players; i++)
        drawColorSnake(i, snakes[i].color);
    checkSnakes();
}

void updateFood(){
    uint64_t appleEaten = 0;
    appleEaten += checkFood(SNAKE1); 
    if(cant_players == 2){
        appleEaten += checkFood(SNAKE2);
    }
    if( appleEaten ){
        eatSound();
        generateFood();
        drawFood();
        appleEaten = 0;
    }
}

void get(){
    char c = getChar();
    if(c == 0) return;
    // hacemos 2 switch porque el if afecta la performance
    switch(c){
            case 'w':
                if(snakes[SNAKE1].dir != DOWN){
                    snakes[SNAKE1].dir = UP;
                }
                break;
            case 's':
                if(snakes[SNAKE1].dir != UP){
                    snakes[SNAKE1].dir = DOWN;
                }
                break;
            case 'a':
                if(snakes[SNAKE1].dir != RIGHT){
                    snakes[SNAKE1].dir = LEFT;
                }
                break;
            case 'd':
                if(snakes[SNAKE1].dir != LEFT){
                    snakes[SNAKE1].dir = RIGHT;
                }
            case 'i':
                if(snakes[SNAKE2].dir != DOWN){
                    snakes[SNAKE2].dir = UP;
                }
            break;
            case 'k':
                if(snakes[SNAKE2].dir != UP){
                    snakes[SNAKE2].dir = DOWN;
                }
            break;
            case 'j':
                if(snakes[SNAKE2].dir != RIGHT){
                    snakes[SNAKE2].dir = LEFT;
                }
            break;
            case 'l':
                if(snakes[SNAKE2].dir != LEFT){
                    snakes[SNAKE2].dir = RIGHT;
                }
                break;
            default:
                break;
        }
}

void moveSnake(snakeID id){
    Point2D tail = snakes[id].body[snakes[id].length - 1];

    for(int i = snakes[id].length - 1; i > 0; i--){
        snakes[id].body[i] = snakes[id].body[i-1];
    }
    switch (snakes[id].dir){
        case UP:
            snakes[id].body[0].y -= SIZE;
            break;
        case DOWN:
            snakes[id].body[0].y += SIZE;
            break;
        case LEFT:
            snakes[id].body[0].x -= SIZE;
            break;
        case RIGHT:
            snakes[id].body[0].x += SIZE;
            break;
    }
    if(snakes[id].addTail){
        snakes[id].body[snakes[id].length] = tail;
        snakes[id].length++;
        snakes[id].addTail = 0;
    }
}

void drawColorSnake(snakeID id, uint32_t color){
    Point2D bottomRight, topLeft;

    for (int i = 0; i < snakes[id].length; i++) {
        topLeft = snakes[id].body[i];
        bottomRight.x = topLeft.x + SIZE;
        bottomRight.y = topLeft.y + SIZE;
        syscall_drawRectangle(&topLeft, &bottomRight, color);
    }
}

int checkPoint(Snake s, Point2D p){
    for(int i = 0 ; i < s.length ; i++){
        if(s.body[i].x == p.x && s.body[i].y == p.y){
            return 0;
        }
    }
    return 1;
}

void generateFood(){
    generated = 0; 
    while(!generated){
        foodPosition.x = SCALE_BY_SIZE((randInt() % (snakeWidth))) + min_X;
        foodPosition.y = SCALE_BY_SIZE((randInt() % (snakeHeight))) + min_Y;
        // Me aseguro que la comida no se superponga con el cuerpo de ninguna serpiente
        if(checkPoint(snakes[SNAKE1], foodPosition) && (cant_players == 1 || checkPoint(snakes[SNAKE2], foodPosition))){
            generated = 1;
        }
    }
}

// Chequea si comio algo
uint64_t checkFood(snakeID id){
    if(snakes[id].body[0].x == foodPosition.x && snakes[id].body[0].y == foodPosition.y){
        snakes[id].addTail = 1;
        return 1;
    }
    return 0;
}

// Dibuja el borde del juego y las serpientes
void setup(){ 
    syscall_drawRectangle(&(Point2D){min_X, min_Y-4}, &(Point2D){max_X, min_Y-1}, WALL_COLOR);
    syscall_drawRectangle(&(Point2D){min_X, max_Y+1}, &(Point2D){max_X, max_Y+4}, WALL_COLOR);
    syscall_drawRectangle(&(Point2D){min_X-4, min_Y}, &(Point2D){min_X-1, max_Y}, WALL_COLOR);
    syscall_drawRectangle(&(Point2D){max_X+1, min_Y}, &(Point2D){max_X+4, max_Y}, WALL_COLOR);

    Point2D startSnake1 = {SCALE_BY_SIZE(snakeWidth/2) + min_X - SIZE, SCALE_BY_SIZE(snakeHeight/2) + min_Y}; 
    setupSnake(SNAKE1, startSnake1, SNAKE1_COLOR, UP);

    if(cant_players == 2){
        Point2D startSnake2 = {SCALE_BY_SIZE(snakeWidth/2) + min_X + SIZE, SCALE_BY_SIZE(snakeHeight/2) + min_Y}; 
        setupSnake(SNAKE2, startSnake2, SNAKE2_COLOR, DOWN);
    }
}

void setupSnake(snakeID id, Point2D start, uint32_t color, direction dir){
    snakes[id].body[0] = start;
    snakes[id].length = 1;
    snakes[id].color = color;
    snakes[id].dir = dir;
    snakes[id].addTail = 0;

}

void drawScores(){
    int player1Score = CALCULATE_POINTS(snakes[SNAKE1].length);
    int player2Score = (cant_players == 2) ? CALCULATE_POINTS(snakes[SNAKE2].length) : 0;
    if(lastScoreLen){
        for(int i=0; i<lastScoreLen; i++){
            printf("\b");
        }
    }
    lastScoreLen = (cant_players == 2 ? printf("P1: %d    P2: %d",player1Score,player2Score): printf("P1: %d",player1Score));
}

void drawFood(){ 
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++){
            syscall_drawRectangle(&(Point2D){foodPosition.x + j, foodPosition.y + i}, &(Point2D){foodPosition.x + j + 1, foodPosition.y + i + 1}, (apple[i][j] == '1') ? FOOD_COLOR : BLACK_COLOR);
        }
    }
}

void checkSnakes(){ 
    gameOver = checkSnakeBounds(SNAKE1) ? (cant_players == 2 ? PLAYER2_WIN : PLAYER1_LOSE) : CONTINUE; 
    if(cant_players == 2 && !gameOver){
        gameOver = checkSnakeBounds(SNAKE2) ? PLAYER1_WIN : 0;
        int collision1 = checkSnakeCollision(snakes[SNAKE1], snakes[SNAKE2]);
        int collision2 = checkSnakeCollision(snakes[SNAKE2], snakes[SNAKE1]);

        if(collision1 && collision2) {
            // Ambos colisionan --> desempate por puntos
            uint64_t points1 = CALCULATE_POINTS(snakes[SNAKE1].length);
            uint64_t points2 = CALCULATE_POINTS(snakes[SNAKE2].length);
            if(points1 > points2){
                gameOver = PLAYER1_WIN;
            } else if(points2 > points1){
                gameOver = PLAYER2_WIN;
            } else { // Misma cantidad de puntos y colisionan cabeza-cabeza
                gameOver = GAME_DRAW;
            }
        } else if(collision1) {
            gameOver = PLAYER2_WIN; // Colisiona 1 con 2 --> gana 2
        } else if(collision2) {
            gameOver = PLAYER1_WIN; // Colisiona 2 con 1 --> gana 1
        }
    }
}

int checkSnakeBounds(snakeID id){
    for(int i=0; i<snakes[id].length; i++){
        if(snakes[id].body[i].x + SIZE > max_X || snakes[id].body[i].x < min_X  
        || snakes[id].body[i].y + SIZE > max_Y || snakes[id].body[i].y < min_Y){
            return 1;
        } 
    }
    for(int i=1; i<snakes[id].length; i++){
        if(snakes[id].body[0].x == snakes[id].body[i].x && snakes[id].body[0].y == snakes[id].body[i].y){
            return 1; 
        }
    }
    return 0;
}

int checkSnakeCollision(Snake mainSnake, Snake targetSnake){
    for(int i=0; i<targetSnake.length; i++){
        if(mainSnake.body[0].x == targetSnake.body[i].x && mainSnake.body[0].y == targetSnake.body[i].y ){
            return 1; // Gana targetSnake
        }
    }
    return 0;
}


void start_screen(){
    syscall_sizeUpFont(2);
    printf("\n\n\n\n\n\n");
    syscall_sizeDownFont(2);

    Point2D bottomLeft;
    Point2D topRight;
    int j, x, y;
    y=0;
    for(int k = 0; k < TITLE_LEN; k++){
        j = x = 0;
        while(snakeTitle[k][j] != 0){
            topRight.x = x;
            topRight.y = y;
            bottomLeft.x = x + TITLE_SIZE;
            bottomLeft.y = y + TITLE_SIZE;
            syscall_drawRectangle(&topRight, &bottomLeft, snakeTitle[k][j] != ' ' ? (snakeTitle[k][j] == '2' ? 0x000CFF00 : 0x00C3FFC0): 0x00000000);
            x += TITLE_SIZE;
            j++;
        }
        y += TITLE_SIZE;
    }
}