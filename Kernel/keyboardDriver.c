extern char kb_getKey();

typedef struct {
    char scan_code;
    char ascii; // if ascii = 0, it is not a printable ascii
} ScanCode;

ScanCode press_keys[] = { // from 0x01 to 0x58. sub 0x81 for release keys
    {},
    {0x01, 0x1B}, // escape
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
    {0x0C, '-'},
    {0x0D, '='},
    {0x0E, 0x08}, // backspace
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

/**
 * @param c char 
 * @returns non-zero if c is a printable ascii, zero if not.
 */
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