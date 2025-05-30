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
    int semaphore_read;  // ID del semáforo para lectores
    int semaphore_write; // ID del semáforo para escritores
    int mutex;           // ID del mutex para acceso exclusivo
    int readers;
    int writers;
    int is_open;
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES];
    int next_pipe_id;
} io_system_t;

// Funciones públicas
void init_io_system();
int create_pipe();
int pipe_read(int pipe_id, char *buffer, int size);
int pipe_write(int pipe_id, const char *buffer, int size);
int close_pipe(int pipe_id);

#endif