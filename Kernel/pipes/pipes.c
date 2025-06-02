#include <pipes.h>
#include <semaphore.h>
#include <stddef.h>

static ioSystem_t ioSystem;

static int next_sem_id = 0;
static int ioSystemInit = 0;

static int checkSystemInit() {
    if (!ioSystemInit) {
        initIoSystem();
        ioSystemInit = 1;
    }
    return 0;
}

void initIoSystem() {
    ioSystem.nextPipeId = 0;
    for(int i = 0; i < MAX_PIPES; i++) {
        ioSystem.pipes[i].isOpen = 0;
        ioSystem.pipes[i].read_pos = 0;
        ioSystem.pipes[i].write_pos = 0;
        ioSystem.pipes[i].count = 0;
        ioSystem.pipes[i].readers = 0;
        ioSystem.pipes[i].writers = 0;
        ioSystem.pipes[i].semReaders = -1;
        ioSystem.pipes[i].semWriters = -1;
        ioSystem.pipes[i].mutex = -1; 
    }
}

int createPipe() {
    checkSystemInit();
    
    for(int i = 0; i < MAX_PIPES; i++) {
        if(!ioSystem.pipes[i].isOpen) {
            pipe_t *pipe = &ioSystem.pipes[i];
            
            pipe->read_pos = 0;
            pipe->write_pos = 0;
            pipe->count = 0;
            pipe->readers = 0;
            pipe->writers = 0;
            pipe->isOpen = 1;
            
            // Crear semáforos usando la API existente
            pipe->semReaders = next_sem_id++;
            pipe->semWriters = next_sem_id++;
            pipe->mutex = next_sem_id++;
            
            // Inicializar semáforos
            if(semInit(pipe->semReaders, 0) < 0 ||           // Sin datos inicialmente
               semInit(pipe->semWriters, PIPE_BUFFER_SIZE) < 0 || // Buffer vacío
               semInit(pipe->mutex, 1) < 0) {                    // Mutex
                return -1;
            }
            
            return i;
        }
    }
    return -1; // No hay pipes disponibles
}

int readPipe(int pipe_id, char *buffer, int size) {
    checkSystemInit();
    
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !ioSystem.pipes[pipe_id].isOpen || buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &ioSystem.pipes[pipe_id];
    int bytes_read = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semReaders); // Esperar datos disponibles
        semWait(pipe->mutex);          // Acceso exclusivo
        
        if(pipe->count > 0) {
            buffer[i] = pipe->buffer[pipe->read_pos];
            pipe->read_pos = (pipe->read_pos + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
            bytes_read++;
        }
        
        semPost(pipe->mutex);
        semPost(pipe->semWriters); // Hay espacio disponible
    }
    
    return bytes_read;
}

int writePipe(int pipe_id, const char *buffer, int size) {
    checkSystemInit();
    
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !ioSystem.pipes[pipe_id].isOpen || buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &ioSystem.pipes[pipe_id];
    int bytes_written = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semWriters); // Esperar espacio disponible
        semWait(pipe->mutex);           // Acceso exclusivo
        
        if(pipe->count < PIPE_BUFFER_SIZE) {
            pipe->buffer[pipe->write_pos] = buffer[i];
            pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
            bytes_written++;
        }
        
        semPost(pipe->mutex);
        semPost(pipe->semReaders); // Datos disponibles
    }
    
    return bytes_written;
}

int closePipe(int pipe_id) {
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !ioSystem.pipes[pipe_id].isOpen)
        return -1;
        
    pipe_t *pipe = &ioSystem.pipes[pipe_id];
    
    // Cerrar semáforos
    semClose(pipe->semReaders);
    semClose(pipe->semWriters);
    semClose(pipe->mutex);
    
    // Limpiar estructura
    pipe->isOpen = 0;
    pipe->read_pos = 0;
    pipe->write_pos = 0;
    pipe->count = 0;
    pipe->readers = 0;
    pipe->writers = 0;
    pipe->semReaders = -1;
    pipe->semWriters = -1;
    pipe->mutex = -1;
    
    return 0;
}