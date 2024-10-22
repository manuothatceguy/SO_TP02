#define BUFFER_SIZE 1000

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

static char buffer[BUFFER_SIZE];

unsigned int shift = 0;
unsigned int caps = 0;
unsigned int mayus = 0;
unsigned int specialKey = 0;

static unsigned int current = 0;
static unsigned int next = 0;

static char isFKey(unsigned int key){
    return (key >= F1_PRESS && key <= F10_PRESS) || key == F11_PRESS || key == F12_PRESS;
}

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
        default:
            break;
    }
}

static void checkSpecialKeys(unsigned int key){
    if(key == LSHIFT_PRESS || key == RSHIFT_PRESS|| key == CAPS_PRESS 
    || key == ALT_PRESS || key == CTRL_PRESS || key == ESC_PRESS || isFKey(key)){
        handleSpecialKeys(key);
        specialKey = 1;
    }
}

static void addToBuffer(unsigned int key){
    if(current < BUFFER_SIZE){
        buffer[current++] = key;
    } else {
        current = 0;
        buffer[current++] = key;
    }
}

void bufferWrite(){
    specialKey = 0;
    int c = kb_getKey();

    checkSpecialKeys(c);
    mayus = (caps && !shift) || (!caps && shift);

    if(!specialKey){
        if( (isAlpha(c) && mayus) || (!isAlpha(c) && shift) ){
            addToBuffer(press_keys[c].shift_ascii);
        } else {
            addToBuffer(press_keys[c].ascii);
        }
    }
}

char getChar(){
    if(next < current){
        return buffer[next++];
    }
    return 0;
}





/*extern char kb_getKey();

typedef struct {
    char scan_code;
    char ascii; // if ascii = 0, it is not a printable ascii
} ScanCode;

ScanCode press_keys[] = { // from 0x01 to 0x58. sub 0x81 for release keys
    {},
    {0x01, '\b'}, // escape
    {0x02, '1'},
    {0x03, '2'},
    {0x04, '3'},
    {0x05, '4'},
    {0x06, '5'},
    {0x07, '6'},
    {0x08, '7'},
    {0x09, '8'},
    {0x0A, '9'},
    {0x0B, '0'},
    {0x0C, '\''},
    {0x0D, '¿'},
    {0x0E, '\b'}, // backspace
    {0x0F, '\t'},
    {0x10, 'q'},
    {0x11, 'w'},
    {0x12, 'e'},
    {0x13, 'r'},
    {0x14, 't'},
    {0x15, 'y'},
    {0x16, 'u'},
    {0x17, 'i'},
    {0x18, 'o'},
    {0x19, 'p'},
    {0x1A, '['},
    {0x1B, ']'},
    {0x1C, '\n'},
    {0x1D, 0}, // left ctrl
    {0x1E, 'a'},
    {0x1F, 's'},
    {0x20, 'd'},
    {0x21, 'f'},
    {0x22, 'g'},
    {0x23, 'h'},
    {0x24, 'j'},
    {0x25, 'k'},
    {0x26, 'l'},
    {0x27, ';'},
    {0x28, '\''},
    {0x29,'`'},
    {0x2A, 0x10}, // left shift
    {0x2C, 'z'},
    {0x2D, 'x'},
    {0x2E, 'c'},
    {0x2F, 'v'},
    {0x30, 'b'},
    {0x31, 'n'},
    {0x32, 'm'},
    {0x33, ','},
    {0x34, '.'},
    {0x35, '/'},
    {0x36, 0x10}, // right shift
    {0x37, '*'}, // keypad *
    {0x38, 0}, // left alt
    {0x39, ' '},
    {0x3A, 0}, // caps lock
    {0x3B, 0}, // F1
    {0x3C, 0}, // F2
    {0x3D, 0}, // F3
    {0x3E, 0}, // F4
    {0x3F, 0}, // F5
    {0x40, 0}, // F6
    {0x41, 0}, // F7
    {0x42, 0}, // F8
    {0x43, 0}, // F9
    {0x44, 0}, // F10
    {0x45, 0}, // NumberLock
    {0x46, 0}, // ScrollLock
    {0x47, '7'}, // keypad 7
    {0x48, '8'}, // keypad 8
    {0x49, '9'}, // keypad 9
    {0x4A, '-'}, // keypad -
    {0x4B, '4'}, // keypad 4
    {0x4C, '5'}, // keypad 5
    {0x4D, '6'}, // keypad 6
    {0x4E, '+'}, // keypad +
    {0x4F, '1'}, // keypad 1
    {0x50, '2'}, // keypad 2
    {0x51, '3'}, // keypad 3
    {0x52, '0'}, // keypad 0
    {0x53, '.'}, // keypad .
    {},
    {},
    {},
    {0x57, 0}, // F11
    {0x58, 0}, // F12
};

char shift_key[][2] = { // sub 33 to get ascii mapped
    {' ', ' '},
    {'!','!'},
    {'\"','\"'},
    {'#','#'},
    {'$','$'},
    {'%','%'},
    {'&','&'},
    {'\'','?'},
    {'(',')'},
    {'*','*'},
    {'+','*'},
    {',',';'},
    {'-','_'},
    {'.',':'},
    {'/','/'},
    {'0','='},
    {'1','!'},
    {'2','"'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','&'},
    {'7','('},
    {'8',')'},
    {'9',')'},
    {':',':'},
    {';',';'},
    {'<','>'},
    {'=','='},
    {'?','?'},
    {'@','@'},
    {'a','A'},
    {'b','B'},
    {'c','C'},
    {'d','D'},
    {'e','E'},
    {'f','F'},
    {'g','G'},
    {'h','H'},
    {'i','I'},
    {'j','J'},
    {'k','K'},
    {'l','L'},
    {'m','M'},
    {'n','N'},
    {'o','O'},
    {'p','P'},
    {'q','Q'},
    {'r','R'},
    {'s','S'},
    {'t','T'},
    {'u','U'},
    {'v','V'},
    {'w','W'},
    {'x','X'},
    {'y','Y'},
    {'z','Z'},
    {'[','['},
    {'\\','\\'},
    {']',']'},
    {'^','^'},
    {'_','_'},
    {'`','`'},
    {'a','a'},
    {'b','b'},
    {'c','c'},
    {'d','d'},
    {'e','e'},
    {'f','f'},
    {'g','g'},
    {'h','h'},
    {'i','i'},
    {'j','j'},
    {'k','k'},
    {'l','l'},
    {'m','m'},
    {'n','n'},
    {'o','o'},
    {'p','p'},
    {'q','q'},
    {'r','r'},
    {'s','s'},
    {'t','t'},
    {'u','u'},
    {'v','v'},
    {'w','w'},
    {'x','x'},
    {'y','y'},
    {'z','z'},
    {'{','['},
    {'|','°'},
    {'}',']'},
    {0x7F, 0x7F} // DEL
};
*/
/**
 * @param c char 
 * @returns non-zero if c is a printable ascii, zero if not.
 */
 /*
char isPrintable(char c){ 
    return c >= 32 && c <= 127;
}

char isShift(char c){
    return c == 0x2A || c == 0x36; // shift keys
}

char getChar(){
    char c = kb_getKey(), shift = 0;
    while(isShift(c)){
        c = kb_getKey();
        shift = 1;
    }
    if(c > 0x80){
        c = kb_getKey();
    }
    return shift_key[press_keys[c].ascii-33][shift];
}
*/