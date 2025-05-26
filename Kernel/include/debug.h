#ifndef _DEBUG_H
#define _DEBUG_H

#include <defs.h>

// Debug flag - set to 1 to enable debug prints, 0 to disable
#define DEBUG_MODE 1

// Default color for debug prints (light blue)
#define DEBUG_COLOR 0x00ADD8E6

// Debug print macro that only prints if DEBUG_MODE is enabled
#define DEBUG_PRINT(msg, color) do { \
    if (DEBUG_MODE) { \
        printStr(msg, 0x00ADD8E6); \
    } \
} while(0)

// Debug print integer macro that only prints if DEBUG_MODE is enabled
#define DEBUG_PRINT_INT(num, color) do { \
    if (DEBUG_MODE) { \
        printInt(num, 0x00ADD8E6); \
    } \
} while(0)

#endif 