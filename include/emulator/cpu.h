#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

// FIX: Force the compiler to pack every byte sequentially with ZERO structural padding gaps
typedef struct __attribute__((packed)) {
    unsigned char  memory[4096];  // 4KB of System RAM
    unsigned char  V[16];         // 16 general-purpose 8-bit registers (V0 - VF)
    unsigned short I;             // 16-bit Index Register
    unsigned short pc;            // 16-bit Program Counter
    unsigned short stack[16];     // Stack for subroutine addresses
    unsigned char  sp;            // Stack Pointer
    unsigned char  delay_timer;   // Delay Timer
    unsigned char  sound_timer;   // Sound Timer
    unsigned char  gfx[64 * 32];  // Display frame matrix
    unsigned char  key[16];       // Keyboard inputs array
    char lastKeyPressed;
} Chip8;

void chip8_init(Chip8* c8);
void chip8_cycle(Chip8* c8);
void chip8_update_timers(Chip8* c8);

#endif
