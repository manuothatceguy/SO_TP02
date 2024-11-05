#include <snakeSound.h>
#include <syscall.h>

void eatSound(){
    syscall_beep(500, 18);
    syscall_beep(700, 18);
}

void deathSound(){  
    syscall_beep(160, 36);
    syscall_beep(100, 36);
}