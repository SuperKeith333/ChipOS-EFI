#include <stdint.h>

#include "emulator/cpu.h"

static const uint8_t chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static uint64_t rand_seed = 88172645463325252ULL;

uint64_t get_random(void) {
    rand_seed ^= (rand_seed << 13);
    rand_seed ^= (rand_seed >> 7);
    rand_seed ^= (rand_seed << 17);
    return rand_seed;
}

_Bool setPixel(unsigned char* display, int x, int y) {
    int wrappedX = x % 64;
    int wrappedY = y % 32;
    
    // check for negative values
    if (wrappedX < 0) wrappedX += 64;
    if (wrappedY < 0) wrappedY += 32;

    int index = wrappedY * 64 + wrappedX;
    _Bool erased = display[index] == 1;
    display[index] ^= 1;
    return erased;
}

void chip8_init(Chip8* c8, uint8_t rom[], uint16_t romSize) {
    for (int i = 0; i < 4096; i++) {
        c8->memory[i] = 0;
    }
    for (int i = 0; i < 16; i++) {
        c8->V[i] = 0;
        c8->stack[i] = 0;
        c8->key[i] = 0;
    }
    for (int i = 0; i < 64 * 32; i++) {
        c8->gfx[i] = 0;
    }

    c8->pc = 0x200; 
    c8->I = 0;
    c8->sp = 0;
    c8->delay_timer = 0;
    c8->sound_timer = 0;
    c8->drewSprite = 0;
    c8->lastKeyPressed = -1;

    for (int i = 0; i < 80; i++) {
        c8->memory[0x050 + i] = chip8_fontset[i];
    }

    
    for (int i = 0; i < romSize; i++) {
        c8->memory[0x200 + i] = rom[i];
    }
}

void chip8_cycle(Chip8* c8) {
    uint16_t instruction = (c8->memory[c8->pc] << 8) | c8->memory[c8->pc + 1];
    c8->pc = c8->pc + 2;

    switch (instruction & 0xF000) {
        case 0x0000:
            switch (instruction & 0x00FF) {
                case 0x00E0:
                    for (int i = 0; i < 2048; i++) {
                        c8->gfx[i] = 0;
                    }
                    break;
                case 0x00EE:
                    c8->sp--;
                    c8->pc = c8->stack[c8->sp];
                    break;
            }
            break;
        case 0x1000: {
            c8->pc = instruction & 0x0FFF;
            break;
        }
        case 0x2000: {
            c8->stack[c8->sp] = c8->pc;
            c8->sp++;
            c8->pc = instruction & 0x0FFF;
            break;
        }
        case 0x3000: {
            if (c8->V[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF)) {
                c8->pc += 2;
            }
            break;
        }
        case 0x4000: {
            if (c8->V[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF)) {
                c8->pc += 2;
            }
            break;
        }
        case 0x5000: {
            if (c8->V[(instruction & 0x0F00) >> 8] == c8->V[(instruction & 0x00F0) >> 4]) {
                c8->pc += 2;
            }
            break;
        }
        case 0x6000: {
            uint8_t x = (instruction & 0x0F00) >> 8;
            c8->V[x] = instruction & 0x00FF;
            break;
        }
        case 0x7000: {
            c8->V[(instruction & 0x0F00) >> 8] = (instruction & 0x00FF) + c8->V[(instruction & 0x0F00) >> 8];
            break;
        }
        case 0x8000: {
            switch (instruction & 0x000F) {
                case 0x0000: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->V[(instruction & 0x00F0) >> 4]; 
                    break;
                }
                case 0x0001: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->V[(instruction & 0x0F00) >> 8] | c8->V[(instruction & 0x00F0) >> 4]; 
                    c8->V[0xF] = 0;
                    break;
                }
                case 0x0002: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->V[(instruction & 0x0F00) >> 8] & c8->V[(instruction & 0x00F0) >> 4];
                    c8->V[0xF] = 0;
                    break;
                }
                case 0x0003: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->V[(instruction & 0x0F00) >> 8] ^ c8->V[(instruction & 0x00F0) >> 4];
                    c8->V[0xF] = 0;
                    break;
                }
                case 0x0004: {
                    uint16_t sum = c8->V[(instruction & 0x0F00) >> 8] + c8->V[(instruction & 0x00F0) >> 4];
                    
                    
                    uint8_t final_val = sum & 0xFF;
                    uint8_t carry = (sum > 255) ? 1 : 0;
                    
                    c8->V[(instruction & 0x0F00) >> 8] = final_val;
                    c8->V[0xF] = carry;
                    break;
                }
                case 0x0005: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    uint8_t y = (instruction & 0x00F0) >> 4;
                    
                    uint8_t val_x = c8->V[x];
                    uint8_t val_y = c8->V[y];
                    
                    uint8_t borrow = (val_x >= val_y) ? 1 : 0;
                    uint8_t final_val = val_x - val_y;

                    c8->V[x] = final_val;
                    c8->V[0xF] = borrow;
                    break;
                }
                case 0x0006: { 
                    uint8_t x = (instruction & 0x0F00) >> 8;

                    uint8_t original_val = c8->V[x]; 
                    
                    // lsb = least significant bit
                    uint8_t lsb = original_val & 0x01;
                    uint8_t final_val = original_val >> 1;

                    c8->V[x] = c8->V[x] >> 1;
                    c8->V[0xF] = lsb;
                    break;
                }

                case 0x0007: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    uint8_t y = (instruction & 0x00F0) >> 4;

                    uint8_t val_x = c8->V[x];
                    uint8_t val_y = c8->V[y];

                    uint8_t borrow = (val_y >= val_x) ? 1 : 0;
                    uint8_t final_val = val_y - val_x;

                    c8->V[x] = final_val;
                    c8->V[0xF] = borrow;
                    break;
                }
                case 0x000E: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    
                    uint8_t original_val = c8->V[x];

                    // msb = most significant bit
                    uint8_t msb = (original_val & 0x80) >> 7;
                    uint8_t final_val = original_val << 1;

                    c8->V[x] = c8->V[x] << 1;
                    c8->V[0xF] = msb;
                    break;
                }
            }
            break;
        }
        case 0x9000: {
            if (c8->V[(instruction & 0x0F00) >> 8] != c8->V[(instruction & 0x00F0) >> 4]) {
                c8->pc += 2;
            }
            break;
        }
        case 0xA000: {
            c8->I = (instruction & 0x0FFF);
            break;
        }
        case 0xB000: {
            c8->pc = (instruction & 0x0FFF) + c8->V[0];
        }
        case 0xC000: {
            uint8_t x = (instruction & 0x0F00) >> 8;
            uint8_t kk = instruction & 0x00FF;

            uint8_t random_byte = (uint8_t)(get_random() & 0xFF);

            c8->V[x] = kk & random_byte;
            break;
        }
        case 0xD000: {
            c8->drewSprite = 1;

            uint8_t x = c8->V[(instruction & 0x0F00) >> 8];
            uint8_t y = c8->V[(instruction & 0x00F0) >> 4];
            uint8_t height = instruction & 0x000F;

            c8->V[0xF] = 0; // reset collision flag
            for (int row = 0; row < height; row++) {
                uint8_t spriteByte = c8->memory[c8->I + row];
                for (int col = 0; col < 8; col++) {
                    if ((spriteByte & (0x80 >> col)) != 0) { // check bit
                        if (setPixel(c8->gfx, x + col, y + row)) {
                            c8->V[0xF] = 1; // collision detected
                        }
                    }
                }
            }
            break;
        }
        case 0xE000: {
            uint16_t x = (instruction & 0x0F00) >> 8;
            uint16_t key_index = c8->V[x] & 0x0F;

            switch (instruction & 0x00FF) {
                case 0x009E: { 
                    if (c8->key[key_index] == 1) {
                        c8->pc += 2;
                    }
                    break;
                }
                case 0x00A1: {
                    if (c8->key[key_index] == 0) {
                        c8->pc += 2;
                    }
                    break;
                }   
            }
            break;
        }
        case 0xF000: {
            switch (instruction & 0x00FF) {
                case 0x000A: {
                    _Bool keyPressed = 0;
                    uint8_t key = 0;
                    for (uint8_t i = 0; i < 16; i++) {
                        if (c8->key[i] == 1) {
                            keyPressed = 1;
                            key = i;
                        }
                    }

                    if (!keyPressed) {
                        if (c8->lastKeyPressed == -1) {
                            c8->pc -= 2;
                        }
                        else if (c8->lastKeyPressed != -1) {
                            c8->lastKeyPressed = -1;
                        }
                    } else {
                        uint8_t x = (instruction & 0x0F00) >> 8;
                        c8->V[x] = key;
                        c8->lastKeyPressed = key;
                        c8->pc -= 2;
                    }
                    break;
                }
                case 0x0007: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->delay_timer;
                    break;
                }
                case 0x0015: {
                    c8->delay_timer = c8->V[(instruction & 0x0F00) >> 8];
                    break;
                }
                case 0x0018: {
                    c8->sound_timer = c8->V[(instruction & 0x0F00) >> 8];
                    break;
                }
                case 0x001E: {
                    c8->I = c8->V[(instruction & 0x0F00) >> 8] + c8->I;
                    break;
                }
                case 0x0029: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    c8->I = 0x50 + (c8->V[x] * 5);
                    break;
                }
                case 0x0033: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    uint8_t value = c8->V[x];

                    c8->memory[c8->I]     = value / 100;         // hundreds
                    c8->memory[c8->I + 1] = (value / 10) % 10;  // tens
                    c8->memory[c8->I + 2] = value % 10;         // ones
                    break;
                }
                case 0x0055: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    for (int i = 0; i <= x; i++) {
                        c8->memory[c8->I] = c8->V[i];
                        c8->I += 1;
                    }
                    break;
                }
                case 0x0065: {
                    uint8_t x = (instruction & 0x0F00) >> 8;
                    for (uint8_t i = 0; i <= x; i++) {
                        c8->V[i] = c8->memory[c8->I];
                        c8->I += 1;
                    }
                    break;
                }
            }
            break;
        }



    }
}

void chip8_update_timers(Chip8* c8) {
    if (c8->delay_timer > 0) {
        c8->delay_timer--;
    }
    if (c8->sound_timer > 0) {
        c8->sound_timer--;
    }
}
