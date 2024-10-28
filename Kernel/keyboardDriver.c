#define BUFFER_SIZE 1000
#define CANT_SPECIAL_KEYS 6

#define LSHIFT_PRESS 0x2A
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_PRESS 0x36
#define RSHIFT_RELEASE 0xB6
#define CAPS_PRESS 0x3A

#define F1_PRESS 0x3B
#define F10_PRESS 0x44
#define F11_PRESS 0x57
#define F12_PRESS 0x58

#define ESC_PRESS 0x1B
#define ALT_PRESS 0x3B
#define CTRL_PRESS 0x1D

extern int kb_getKey();

typedef struct {
    char ascii;
    char shift_ascii; 
} ScanCode;

ScanCode press_keys[] = { // from 0x01 to 0x58. sub 0x81 for release keys
    {0,0},
    {0x1B, 0x1B}, // escape
    {'1','!'},
    {'2','"'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','&'},
    {'7','('},
    {'8',')'},
    {'9',')'},
    {'0','='},
    {'-','_'},
    {'=', '+'}, // shift value corrected
    {0x08, 0x08}, // backspace
    {'\t', '\t'}, // tab
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'}, // shift for brackets
    {']', '}'},
    {'\n', '\n'}, // enter
    {0, 0}, // left ctrl (no ASCII)
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'}, // shift for semicolon
    {'\'', '\"'}, // shift for single quote
    {'`', '~'}, // shift for backtick
    {0, 0}, // left shift
    {'\\', '|'}, // shift for backslash NASH??
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'}, // shift for comma
    {'.', '>'}, // shift for period
    {'/', '?'}, // shift for slash
    {0, 0}, // right shift
    {'*', '*'}, // keypad *
    {0, 0}, // left alt (no ASCII)
    {' ', ' '}, // space
    {0, 0}, // caps lock (no ASCII)
    {0, 0}, // F1
    {0, 0}, // F2
    {0, 0}, // F3
    {0, 0}, // F4
    {0, 0}, // F5
    {0, 0}, // F6
    {0, 0}, // F7
    {0, 0}, // F8
    {0, 0}, // F9
    {0, 0}, // F10
    {0, 0}, // NumberLock (no ASCII)
    {0, 0}, // ScrollLock (no ASCII)
    {'7', '7'}, // keypad 7
    {'8', '8'}, // keypad 8
    {'9', '9'}, // keypad 9
    {'-', '_'}, // keypad -
    {'4', '4'}, // keypad 4
    {'5', '5'}, // keypad 5
    {'6', '6'}, // keypad 6
    {'+', '+'}, // keypad +
    {'1', '1'}, // keypad 1
    {'2', '2'}, // keypad 2
    {'3', '3'}, // keypad 3
    {'0', '0'}, // keypad 0
    {'.', '>'}, // keypad .
    {0, 0}, // empty entries
    {0, 0},
    {0, 0},
    {0, 0}, // F11
    {0, 0}  // F12
};

static unsigned int specialKeys[] = {LSHIFT_PRESS, LSHIFT_RELEASE, RSHIFT_RELEASE, RSHIFT_PRESS, CAPS_PRESS, ALT_PRESS, CTRL_PRESS, ESC_PRESS};
static char buffer[BUFFER_SIZE];

unsigned int shift = 0;
unsigned int caps = 0;
unsigned int mayus = 0;
unsigned int esc = 0;
unsigned int specialKey = 0;

static unsigned int current = 0;
static unsigned int next = 0;

/*
static char isFKey(unsigned int key){
    return (key >= F1_PRESS && key <= F10_PRESS) || key == F11_PRESS || key == F12_PRESS;
}
*/

static char isAlpha(unsigned int key){
    return press_keys[key].ascii >= 'a' && press_keys[key].ascii <= 'z';
}


static void handleSpecialKeys(unsigned int key){
    switch(key){
        case LSHIFT_PRESS:
        case RSHIFT_PRESS:
            shift = 1;
            break;
        case LSHIFT_RELEASE:
        case RSHIFT_RELEASE:
            shift = 0;
            break;
        case CAPS_PRESS:
            caps = !caps;
            break;
        case ESC_PRESS:
            esc = 1; 
        default:
            break;

        if(esc){
            getRegisters(); // Funcion de ASM. VER 
            esc = 0; 
        }
        
    }
}


static void checkSpecialKeys(unsigned int key){
    for(int i = 0; i < CANT_SPECIAL_KEYS && !specialKey ; i++){
        if(key == specialKeys[i]){
            handleSpecialKeys(key);
            specialKey = 1;
        }
    }
    /*                  O ES MEJOR ESTO ??? @mothatceguy

    if(key == LSHIFT_PRESS || key == RSHIFT_PRESS|| key == CAPS_PRESS 
    || key == ALT_PRESS || key == CTRL_PRESS || key == ESC_PRESS || isFKey(key) 
    || key == BACKSPACE_PRESS || key == TAB_PRESS || key == ENTER_PRESS){ 
        handleSpecialKeys(key);
        specialKey = 1;
    }  


    */
}

static void addToBuffer(unsigned int key){
    current %= BUFFER_SIZE;
    next %= BUFFER_SIZE;
    buffer[current++] = key;
}

int bufferWrite(){
    specialKey = 0;
    int c = kb_getKey();

    checkSpecialKeys(c);

    if(!specialKey && c <= F12_PRESS){  // A partir de F12_PRESS tengo los release keys
        mayus = (caps && !shift) || (!caps && shift);
        if( (isAlpha(c) && mayus) || (!isAlpha(c) && shift) ){
            addToBuffer(press_keys[c].shift_ascii);
        } else {
            addToBuffer(press_keys[c].ascii);
        }
        return 1;
    }
    return 0;
}

char getChar(){
    if(next < current){
        return buffer[next++];
    }
    return 0;
}