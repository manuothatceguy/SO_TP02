#ifndef TEXTMODULE_H
#define TEXTMODULE_H

#include <videoDriver.h>

void loadFont(char **newFont, int newFontHeight, int newFontWidth);

void putChar(unsigned char c, int scaleFactor, uint32_t color);

void lineFeed(int fontHeight);

void printStr(char * s, uint32_t color);

void printStrSize(char * s, uint32_t color, int scaleFactor);

#endif // TEXTMODULE_H