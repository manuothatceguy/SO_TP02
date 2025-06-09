#ifndef PIPES_H
#define PIPES_H

#include <semaphore.h>

#define MAX_PIPES 16
#define PIPE_BUFFER_SIZE 100

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];
    int readIdx;
    int writeIdx;
    int count;
    int semReaders;  // ID del semáforo para lectores
    int semWriters; // ID del semáforo para escritores
    int mutex;           // ID del mutex para acceso exclusivo
    int readers;
    int writers;
    int isOpen;
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES];
    int next_pipe_id;
} pipeManager;

// Funciones públicas
void initPipeManager();
int createPipe();
int readPipe(int pipe_id, char *buffer, int size);
int writePipe(int pipe_id, const char *buffer, int size);
int closePipe(int pipe_id);
int clearPipe(int pipe_id);

#endif