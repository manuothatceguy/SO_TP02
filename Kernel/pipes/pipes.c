#include <pipes.h>
#include <semaphore.h>
#include <stddef.h>

static io_system_t io_system;

// Variable global para gestión de IDs de semáforos
static int next_sem_id = 0;
static int io_system_initialized = 0;

// Función auxiliar para verificar si el sistema está inicializado
static int ensure_io_system_initialized() {
    if (!io_system_initialized) {
        init_io_system();
        io_system_initialized = 1;
    }
    return 0;
}

void init_io_system() {
    io_system.next_pipe_id = 0;
    for(int i = 0; i < MAX_PIPES; i++) {
        io_system.pipes[i].is_open = 0;
        io_system.pipes[i].read_pos = 0;
        io_system.pipes[i].write_pos = 0;
        io_system.pipes[i].count = 0;
        io_system.pipes[i].readers = 0;
        io_system.pipes[i].writers = 0;
        io_system.pipes[i].semaphore_read = -1;
        io_system.pipes[i].semaphore_write = -1;
        io_system.pipes[i].mutex = -1; 
    }
}

int create_pipe() {
    ensure_io_system_initialized();
    
    for(int i = 0; i < MAX_PIPES; i++) {
        if(!io_system.pipes[i].is_open) {
            pipe_t *pipe = &io_system.pipes[i];
            
            pipe->read_pos = 0;
            pipe->write_pos = 0;
            pipe->count = 0;
            pipe->readers = 0;
            pipe->writers = 0;
            pipe->is_open = 1;
            
            // Crear semáforos usando la API existente
            pipe->semaphore_read = next_sem_id++;
            pipe->semaphore_write = next_sem_id++;
            pipe->mutex = next_sem_id++;
            
            // Inicializar semáforos
            if(semInit(pipe->semaphore_read, 0) < 0 ||           // Sin datos inicialmente
               semInit(pipe->semaphore_write, PIPE_BUFFER_SIZE) < 0 || // Buffer vacío
               semInit(pipe->mutex, 1) < 0) {                    // Mutex
                return -1;
            }
            
            return i;
        }
    }
    return -1; // No hay pipes disponibles
}

int pipe_read(int pipe_id, char *buffer, int size) {
    ensure_io_system_initialized();
    
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !io_system.pipes[pipe_id].is_open || buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &io_system.pipes[pipe_id];
    int bytes_read = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semaphore_read); // Esperar datos disponibles
        semWait(pipe->mutex);          // Acceso exclusivo
        
        if(pipe->count > 0) {
            buffer[i] = pipe->buffer[pipe->read_pos];
            pipe->read_pos = (pipe->read_pos + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
            bytes_read++;
        }
        
        semPost(pipe->mutex);
        semPost(pipe->semaphore_write); // Hay espacio disponible
    }
    
    return bytes_read;
}

int pipe_write(int pipe_id, const char *buffer, int size) {
    ensure_io_system_initialized();
    
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !io_system.pipes[pipe_id].is_open || buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &io_system.pipes[pipe_id];
    int bytes_written = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semaphore_write); // Esperar espacio disponible
        semWait(pipe->mutex);           // Acceso exclusivo
        
        if(pipe->count < PIPE_BUFFER_SIZE) {
            pipe->buffer[pipe->write_pos] = buffer[i];
            pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
            bytes_written++;
        }
        
        semPost(pipe->mutex);
        semPost(pipe->semaphore_read); // Datos disponibles
    }
    
    return bytes_written;
}

int close_pipe(int pipe_id) {
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !io_system.pipes[pipe_id].is_open)
        return -1;
        
    pipe_t *pipe = &io_system.pipes[pipe_id];
    
    // Cerrar semáforos
    semClose(pipe->semaphore_read);
    semClose(pipe->semaphore_write);
    semClose(pipe->mutex);
    
    // Limpiar estructura
    pipe->is_open = 0;
    pipe->read_pos = 0;
    pipe->write_pos = 0;
    pipe->count = 0;
    pipe->readers = 0;
    pipe->writers = 0;
    pipe->semaphore_read = -1;
    pipe->semaphore_write = -1;
    pipe->mutex = -1;
    
    return 0;
}

// API de usuario para lectura de teclado
int kb_read_char() {
    // Usar pipe 0 reservado para teclado, o el pipe específico del teclado
    int pipe_id = 0; // Asumimos que el pipe 0 está reservado para teclado
    char c;
    if(pipe_read(pipe_id, &c, 1) == 1) {
        return c;
    }
    return -1;
}

int kb_read_line(char *buffer, int max_size) {
    if(buffer == NULL || max_size <= 0) return -1;
    
    int i = 0;
    char c;
    
    while(i < max_size - 1) {
        c = kb_read_char();
        if(c == -1) continue;
        
        buffer[i++] = c;
        if(c == '\n') break;
    }
    
    buffer[i] = '\0';
    return i;
}

int kb_read_available() {
    // Verificar si hay datos disponibles sin bloquear
    int pipe_id = 0;
    if(pipe_id < 0 || pipe_id >= MAX_PIPES || !io_system.pipes[pipe_id].is_open)
        return 0;
        
    return io_system.pipes[pipe_id].count > 0;
}