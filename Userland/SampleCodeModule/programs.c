// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <programs.h>

#define COL_PROCESS 11
#define COL_PID     4
#define COL_STATE   10
#define COL_PRIO    4
#define COL_PPID    5
#define COL_WAIT    5
#define COL_FG      4
#define LINE_WIDTH 65

static void padRight(const char *str, int width) {
    printf("%s", str);
    int len = strlen(str);
    for (int i = len; i < width; i++) printf(" ");
}

static void padInt(int value, int width) {
    char buf[12];
    intToStr(value, buf);
    padRight(buf, width);
}

void printHeader() {
    printf("\nPROCESS    PID STATE     PRIO PPID WAIT FG      REGISTERS\n");
    printf("================================================================\n");
}

void printProcessInfo(PCB processInfo) {
    const char* state;
    switch(processInfo.state) {
        case READY: state = "READY"; break;
        case RUNNING: state = "RUNNING"; break;
        case BLOCKED: state = "BLOCKED"; break;
        case ZOMBIE: state = "ZOMBIE"; break;
        case EXITED: state = "EXITED"; break;
        case KILLED: state = "KILLED"; break;
        case WAITING_SEM: state = "WAITING"; break;
        default: state = "UNKNOWN"; break;
    }

    for (int i = 0; i < LINE_WIDTH - 19; i++) printf(" ");
    printf("RSP: 0x%x\n", (unsigned int)processInfo.rsp);

    padRight(processInfo.name, COL_PROCESS);            // PROCESS (11)
    padInt(processInfo.pid, COL_PID);                   // PID (4)
    padRight(state, COL_STATE);                         // STATE (10)
    padInt(processInfo.priority, COL_PRIO);             // PRIO (4)

    char *ppidStr = processInfo.parentPid == -1 ? "none" : itoa(processInfo.parentPid);
    padRight(ppidStr, COL_PPID);                        // PPID (5)

    char *wpidStr = processInfo.waitingForPid == -1 ? "none" : itoa(processInfo.waitingForPid);
    padRight(wpidStr, COL_WAIT);                        // WAIT (5)

    padRight(processInfo.foreground ? "yes" : "no", COL_FG); // FG (4)

    printf("   RBP: 0x%x\n", (unsigned int)processInfo.base); // REGISTERS

    for (int i = 0; i < LINE_WIDTH - 19; i++) printf(" ");
    printf("Entry: 0x%x\n", (unsigned int)processInfo.entryPoint);
}

uint64_t readLine(char *buff, uint64_t length) {
    int k = 0;
    while(buff[k] != '\0'){buff[k] = '\0'; k++;};
    char c;
    int i = 0;
    while ((c = getChar()) != '\n' && i < length - 1) {
        if (c == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (c != 0) {
            buff[i++] = c;
            printf("%c", c);
        }
    }
    buff[i] = 0;
    printf("\n");
    return i;
}

static int is_space(char c){
    return c == ' ';
}

int parse_string(char *arg, char **args, int max_args, int max_size) {
    if (arg == NULL || arg[0] == '\0') {
        return -1;
    }

    int arg_count = 0;
    int i = 0;

    // Saltar espacios iniciales
    while (is_space(arg[i])) i++;

    while (arg[i] != '\0' && arg_count < max_args) {
        int j = 0;

        // Copiar argumento mientras no haya espacio y haya espacio en buffer
        while (arg[i] != '\0' && !is_space(arg[i]) && j < max_size - 1) {
            args[arg_count][j++] = arg[i++];
        }

        // Terminar string
        args[arg_count][j] = '\0';

        // Si el argumento era más largo, saltar el resto
        while (arg[i] != '\0' && !is_space(arg[i])) {
            i++;
        }

        if (j > 0) {
            arg_count++;
        }

        // Saltar espacios entre argumentos
        while (is_space(arg[i])) i++;
    }

    return arg_count;
}

int anal_arg(char *arg, char **args, int expected_args, int max_size) {
    if (arg == NULL || arg[0] == '\0' ) {
        if(expected_args == 0) return 0;
        return -1;
    }

    int has_background = 0;

    // Copia temporal
    char buffer[1024];
    strncpy(buffer, arg, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Parsear argumentos (permitimos uno más por si hay un & suelto)
    char *temp_args[expected_args + 2];
    char temp_storage[expected_args + 2][max_size];
    for (int i = 0; i < expected_args + 2; i++) {
        temp_args[i] = temp_storage[i];
    }

    int num_args = parse_string(buffer, temp_args, expected_args + 2, max_size);
    if (num_args < 0) return -1;

    // Detectar "&" como argumento independiente
    if (num_args > 0 && strcmp(temp_args[num_args - 1], "&") == 0) {
        has_background = 1;
        num_args--;  // Eliminar '&' del conteo
    }

    if (num_args != expected_args) {
        return -1;
    }

    // Copiar argumentos válidos al arreglo destino
    for (int i = 0; i < expected_args; i++) {
        strncpy(args[i], temp_args[i], max_size);
        args[i][max_size - 1] = '\0';
    }

    return has_background;
}

uint64_t cat(uint64_t argc, char *argv[]) {
    int c;
    char buffer[BUFFER_SPACE] = {0};
    int i = 0;
        
    c = getChar();
    while (c != EOF) {
        if (c != 0) {
            printf("%c", c);
            buffer[i++] = c;
            if (c == '\n') {
                buffer[i] = '\0';
                printf("%s", buffer);
                buffer[0] = '\0';
                i = 0;
            }
        }
        c = getChar();
    }
    printf("\n");
    return 0;
}

void loop(uint64_t argc, char *argv[]) {
    pid_t pid = syscall_getpid();
    // Convertir el argumento a número
    int32_t time = satoi(argv[0]);
    
    if (time <= 0) {
        printf("Error: time debe ser mayor que 0\n");
        return;
    }
    printf("Proceso loop %d iniciado. Saludando cada %d segundos...\n", pid, time);
    
    while(1) {
        printf("\nHola! Soy el proceso %d\n", pid);
        syscall_wait(time);
    }
}



uint64_t wc(uint64_t argc, char *argv[]) {
    int lines = 1;
    int c;
    
    c = getChar();
    while (c != EOF) {
        if (c != 0) {
            if (c == '\n') {
                lines++;
            }
            printf("%c", c);
        }
        c = getChar();
    }
    printf("\n");
    
    printf("Cantidad de lineas: %d\n\n", --lines);
    return 0;
}

uint64_t filter(uint64_t argc, char *argv[]) {
    int c;
    char filtered[BUFFER_SPACE] = {0};
    int i = 0;
    
    c = getChar();
    while (c != EOF) {
        if (c != 0) {
            printf("%c", c);
            if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
                c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U') {
                filtered[i++] = c;
            }
        }
        c = getChar();
    }
    printf("\n%s\n", filtered);
    return 0;
}