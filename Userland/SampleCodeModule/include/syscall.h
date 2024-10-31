#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef struct Point2D {
    uint64_t x, y;
} Point2D;

uint64_t syscall_read(char *buff, uint64_t len);
uint64_t syscall_write(uint64_t fd, char *buff, uint64_t len);
uint64_t syscall_time(uint64_t mod);
uint64_t syscall_beep(uint64_t freq, uint64_t ticks);
uint64_t syscall_drawRectangle(Point2D * ul, Point2D * br, uint32_t color);
uint64_t syscall_getRegisters(uint64_t *registers);
uint64_t syscall_clearScreen();
uint64_t syscall_sizeUpFont(uint64_t increment);
uint64_t syscall_sizeDownFont(uint64_t decrement);
uint64_t syscall_getHeight();
uint64_t syscall_getWidth();
uint64_t syscall_wait(uint64_t ticks);

#endif