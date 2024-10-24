#include <syscall.h>

uint64_t _syscall(uint64_t code, uint64_t rbx, uint64_t rcx, uint64_t rdx);

uint64_t syscall(uint64_t code, int count,...){
    va_list ap;
    va_start(ap, count);
    uint64_t arguments[3] = {0};
    for(int i = 0; i < count; i++){
        arguments[i] = va_arg(ap, uint64_t);
    }
    va_end(ap);
    return _syscall(code, arguments[0], arguments[1], arguments[2]);
}