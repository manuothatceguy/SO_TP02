#include <pipes.h>
#include <semaphore.h>
#include <stddef.h>

static pipeManager pipes;

static int next_sem_id = 0;
static int pipeManagerInit = 0;

// Función auxiliar para verificar si el sistema está inicializado
static int ensurePipeManagerInit() {
    if (!pipeManagerInit) {
        initPipeManager();
        pipeManagerInit = 1;
    }
    return 0;
}

void initPipeManager() {
    pipes.next_pipe_id = 0;
    for(int i = 0; i < MAX_PIPES; i++) {
        pipes.pipes[i].isOpen = 0;
        pipes.pipes[i].readIdx = 0;
        pipes.pipes[i].writeIdx = 0;
        pipes.pipes[i].count = 0;
        pipes.pipes[i].readers = 0;
        pipes.pipes[i].writers = 0;
        pipes.pipes[i].semReaders = -1;
        pipes.pipes[i].semWriters = -1;
        pipes.pipes[i].mutex = -1; 
    }
}

int createPipe() {
    ensurePipeManagerInit();
    
    for(int i = 0; i < MAX_PIPES; i++) {
        if(!pipes.pipes[i].isOpen) {
            pipe_t *pipe = &pipes.pipes[i];
            
            pipe->readIdx = 0;
            pipe->writeIdx = 0;
            pipe->count = 0;
            pipe->readers = 0;
            pipe->writers = 0;
            pipe->isOpen = 1;
            
            pipe->semReaders = next_sem_id++;
            pipe->semWriters = next_sem_id++;
            pipe->mutex = next_sem_id++;
            
            if(semInit(pipe->semReaders, 0) < 0 ||           
               semInit(pipe->semWriters, PIPE_BUFFER_SIZE) < 0 || 
               semInit(pipe->mutex, 1) < 0) {                    
                return -1;
            }
            
            return i;
        }
    }
    return -1; // No hay pipes disponibles
}

int readPipe(int pipe_id, char *buffer, int size) {
    ensurePipeManagerInit();
    
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !pipes.pipes[pipe_id].isOpen || buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &pipes.pipes[pipe_id];
    int bytes_read = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semReaders); // Esperar datos disponibles
        semWait(pipe->mutex);          // Acceso exclusivo
        
        if(pipe->count > 0) {
            buffer[i] = pipe->buffer[pipe->readIdx];
            pipe->readIdx = (pipe->readIdx + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
            bytes_read++;
        }
        
        semPost(pipe->mutex);
        semPost(pipe->semWriters); // Hay espacio disponible
    }
    
    return bytes_read;
}

int writePipe(int pipe_id, const char *buffer, int size) {
    ensurePipeManagerInit();
    
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !pipes.pipes[pipe_id].isOpen || buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &pipes.pipes[pipe_id];
    int bytes_written = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semWriters); // Esperar espacio disponible
        semWait(pipe->mutex);           // Acceso exclusivo
        
        if(pipe->count < PIPE_BUFFER_SIZE) {
            pipe->buffer[pipe->writeIdx] = buffer[i];
            pipe->writeIdx = (pipe->writeIdx + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
            bytes_written++;
        }
        
        semPost(pipe->mutex);
        semPost(pipe->semReaders); // Datos disponibles
    }
    
    return bytes_written;
}

int closePipe(int pipe_id) {
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !pipes.pipes[pipe_id].isOpen)
        return -1;
        
    pipe_t *pipe = &pipes.pipes[pipe_id];
    
    // Cerrar semáforos
    semClose(pipe->semReaders);
    semClose(pipe->semWriters);
    semClose(pipe->mutex);
    
    // Limpiar estructura
    pipe->isOpen = 0;
    pipe->readIdx = 0;
    pipe->writeIdx = 0;
    pipe->count = 0;
    pipe->readers = 0;
    pipe->writers = 0;
    pipe->semReaders = -1;
    pipe->semWriters = -1;
    pipe->mutex = -1;
    
    return 0;
}

int clearPipe(int pipeId){
    if(pipeId < 0 || pipeId >= MAX_PIPES || !pipes.pipes[pipeId].isOpen)
        return -1;
        
    pipe_t *pipe = &pipes.pipes[pipeId];
    
    semWait(pipe->mutex);
    
    pipe->readIdx = 0;
    pipe->writeIdx = 0;
    pipe->count = 0;
    
    semPost(pipe->mutex);
    
    return 0;
}