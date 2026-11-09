#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include <stdint.h>

typedef struct {
    uint8_t  memory[4096];  
    uint8_t  V[16];  // Normal Registers (8-bit)
    uint16_t I; // Index Register (16-bit)
    uint16_t pc;  // Program Counter
    uint16_t stack[16]; 
    uint8_t  sp; // Stack Pointer
    uint8_t  delay_timer;   // Delay Timer
    uint8_t  sound_timer;   // Sound Timer
    uint8_t  gfx[64 * 32];  // Display
    uint8_t  key[16]; // Keyboard inputs
    int lastKeyPressed;
    _Bool drewSprite;
} Chip8;

void chip8_init(Chip8* c8, uint8_t rom[], uint16_t romSize);
void chip8_cycle(Chip8* c8);
void chip8_update_timers(Chip8* c8);

#endif
