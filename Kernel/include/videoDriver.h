#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y);

void penUp();
void penDown();

void setColor(uint32_t color);

int left(uint64_t delta);
int right(uint64_t delta);
int up(uint64_t delta);
int down(uint64_t delta);

int setX(uint64_t newX);
int setY(uint64_t newY);

#endif // VIDEO_DRIVER_H
