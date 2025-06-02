#ifndef _PIPES_H
#define _PIPES_H

#include <semaphore.h>

#define MAX_PIPES 16
#define PIPE_BUFFER_SIZE 4096

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];
    int read_pos;
    int write_pos;
    int count;
    int semReaders;  
    int semWriters; 
    int mutex;           
    int readers;
    int writers;
    int isOpen;
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES];
    int nextPipeId;
} ioSystem_t;

// Funciones públicas
void initIoSystem();
int createPipe();
int readPipe(int pipe_id, char *buffer, int size);
int writePipe(int pipe_id, const char *buffer, int size);
int closePipe(int pipe_id);

#endif