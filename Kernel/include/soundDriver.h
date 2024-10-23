#ifndef SOUND_DRIVER_H
#define SOUND_DRIVER_H

#include <stdint.h>

void play_sound(uint32_t nFrequence);
void nosound();
void beep();

#endif // SOUND_DRIVER_H