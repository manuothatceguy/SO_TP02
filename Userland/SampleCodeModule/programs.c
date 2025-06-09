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

    // Línea superior: RSP
    for (int i = 0; i < LINE_WIDTH - 19; i++) printf(" ");
    printf("RSP: 0x%x\n", (unsigned int)processInfo.rsp);

    // Línea principal
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

    // Línea inferior: EntryPoint
    for (int i = 0; i < LINE_WIDTH - 19; i++) printf(" ");
    printf("Entry: 0x%x\n", (unsigned int)processInfo.entryPoint);
}

int parse_string(char *arg, char **args, int max_args, int max_size) {
    if (arg == NULL || arg[0] == '\0') {
        return -1;
    }
    
    int arg_count = 0;
    int current_pos = 0;
    int start_pos = 0;
    
    // Inicializar todos los argumentos como strings vacíos
    for (int i = 0; i < max_args; i++) {
        args[i][0] = '\0';
    }
    
    // Procesar cada carácter
    while (arg[current_pos] != '\0' && arg_count < max_args) {
        // Si encontramos un espacio o el final de la cadena
        if (arg[current_pos] == ' ' || arg[current_pos + 1] == '\0') {
            // Si es el final de la cadena, incluir el último carácter
            int end_pos = (arg[current_pos + 1] == '\0') ? current_pos + 1 : current_pos;
            
            // Copiar el argumento actual
            int j = 0;
            for (int i = start_pos; i < end_pos && j < max_size - 1; i++) {
                args[arg_count][j++] = arg[i];
            }
            args[arg_count][j] = '\0';
            
            // Solo incrementar arg_count si el argumento no está vacío
            if (j > 0) {
                arg_count++;
            }
            
            start_pos = current_pos + 1;
        }
        current_pos++;
    }
    
    return arg_count;
}

char *months[] = {
    "Enero",
    "Febrero",
    "Marzo",
    "Abril",
    "Mayo",
    "Junio",
    "Julio",
    "Agosto",
    "Septiembre",
    "Octubre",
    "Noviembre",
    "Diciembre"
};

void showTime(){
    uint64_t time[] = {
        syscall_time(0), // secs
        syscall_time(1), // mins
        syscall_time(2), // hours
        syscall_time(3), // day
        syscall_time(4), // month
        syscall_time(5)  // year
    };
    char s[3] = {'0' + time[0] /10 % 10,'0' + time[0] % 10, 0};
    char m[3] = {'0' + time[1] /10 % 10, '0' + time[1] % 10, 0}; 
    char h[3] = {'0' + time[2] /10 % 10, '0' + time[2] % 10, 0};
    printf("Son las %s:%s:%s del %d de %s del %d\n", h, m, s, time[3], months[time[4]-1], time[5]);
}

void showRegisters(){    
    char * registersNames[CANT_REGISTERS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: ", "CS: "};
    uint64_t registersRead[CANT_REGISTERS];
    syscall_getRegisters(registersRead); 
    uint64_t aux = registersRead[7]; // asumiendo RSP [7] distinto de 0
    if(!aux){
        printf("No hay un guardado de registros. Presione ESC para hacer un backup\n");
        return;
    }
    for(int i = 0; i < CANT_REGISTERS ; i++){
        printf("Valor del registro %s %x \n", registersNames[i] , registersRead[i]);
    }
}

uint64_t cat(uint64_t argc, char *argv[]) {
    int c;
    char buffer[BUFFER_SPACE] = {0};
    int i = 0;
    
    //printf("Ingrese el texto (presione Ctrl+D para terminar):\n");
    
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
    uint32_t time = satoi(argv[0]);
    
    if (time <= 0) {
        printf("Error: time debe ser mayor que 0\n");
        return;
    }
    printf("Proceso loop %d iniciado. Saludando cada %d segundos...\n", pid, time);
    
    while(1) {
        printf("Hola! Soy el proceso %d\n", pid);
        syscall_wait(time);
    }
}



uint64_t wc(uint64_t argc, char *argv[]) {
    int lines = 1;
    int c;
    
    //printf("Ingrese el texto (presione Ctrl+D para terminar):\n");
    
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
    
    //printf("Ingrese el texto (presione Ctrl+D para terminar):\n");
    
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