#include <syscall.h>

extern u_int64_t _syscall(u_int64_t code, u_int64_t rbx, u_int64_t rcx, u_int64_t rdx);

u_int64_t syscall(u_int64_t code, int count,...){
    va_list ap;
    va_start(ap, count);
    u_int64_t arguments[3] = {0};
    for(int i = 0; i < count; i++){
        arguments[i] = va_arg(ap, u_int64_t);
    }
    va_end(ap);
    return _syscall(code, arguments[0], arguments[1], arguments[2]);
}