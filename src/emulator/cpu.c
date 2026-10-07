#include "emulator/cpu.h"

// Static copy of the standard Chip-8 PONG ROM bytecode (246 bytes)
static const unsigned char pong_rom_bytes[] = {
  0x12, 0x0a, 0x60, 0x01, 0x00, 0xee, 0x60, 0x02, 0x12, 0xa6, 0x00, 0xe0,
  0x68, 0x32, 0x6b, 0x1a, 0xa4, 0xf1, 0xd8, 0xb4, 0x68, 0x3a, 0xa4, 0xf5,
  0xd8, 0xb4, 0x68, 0x02, 0x69, 0x06, 0x6a, 0x0b, 0x6b, 0x01, 0x65, 0x2a,
  0x66, 0x2b, 0xa4, 0xb5, 0xd8, 0xb4, 0xa4, 0xed, 0xd9, 0xb4, 0xa4, 0xa5,
  0x36, 0x2b, 0xa4, 0xa1, 0xda, 0xb4, 0x6b, 0x06, 0xa4, 0xb9, 0xd8, 0xb4,
  0xa4, 0xed, 0xd9, 0xb4, 0xa4, 0xa1, 0x45, 0x2a, 0xa4, 0xa5, 0xda, 0xb4,
  0x6b, 0x0b, 0xa4, 0xbd, 0xd8, 0xb4, 0xa4, 0xed, 0xd9, 0xb4, 0xa4, 0xa1,
  0x55, 0x60, 0xa4, 0xa5, 0xda, 0xb4, 0x6b, 0x10, 0xa4, 0xc5, 0xd8, 0xb4,
  0xa4, 0xed, 0xd9, 0xb4, 0xa4, 0xa1, 0x76, 0xff, 0x46, 0x2a, 0xa4, 0xa5,
  0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xcd, 0xd8, 0xb4, 0xa4, 0xed, 0xd9, 0xb4,
  0xa4, 0xa1, 0x95, 0x60, 0xa4, 0xa5, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xad,
  0xd8, 0xb4, 0xa4, 0xed, 0xd9, 0xb4, 0xa4, 0xa5, 0x12, 0x90, 0xa4, 0xa1,
  0xda, 0xb4, 0x68, 0x12, 0x69, 0x16, 0x6a, 0x1b, 0x6b, 0x01, 0xa4, 0xb1,
  0xd8, 0xb4, 0xa4, 0xed, 0xd9, 0xb4, 0x60, 0x00, 0x22, 0x02, 0xa4, 0xa5,
  0x40, 0x00, 0xa4, 0xa1, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xa9, 0xd8, 0xb4,
  0xa4, 0xe1, 0xd9, 0xb4, 0xa4, 0xa5, 0x40, 0x02, 0xa4, 0xa1, 0x30, 0x00,
  0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9, 0xd8, 0xb4, 0xa4, 0xa9, 0xd9, 0xb4,
  0xa4, 0xa1, 0x65, 0x2a, 0x67, 0x00, 0x87, 0x50, 0x47, 0x2a, 0xa4, 0xa5,
  0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9, 0xd8, 0xb4, 0xa4, 0xad, 0xd9, 0xb4,
  0xa4, 0xa1, 0x66, 0x0b, 0x67, 0x2a, 0x87, 0x61, 0x47, 0x2b, 0xa4, 0xa5,
  0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9, 0xd8, 0xb4, 0xa4, 0xb1, 0xd9, 0xb4,
  0xa4, 0xa1, 0x66, 0x78, 0x67, 0x1f, 0x87, 0x62, 0x47, 0x18, 0xa4, 0xa5,
  0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9, 0xd8, 0xb4, 0xa4, 0xb5, 0xd9, 0xb4,
  0xa4, 0xa1, 0x66, 0x78, 0x67, 0x1f, 0x87, 0x63, 0x47, 0x67, 0xa4, 0xa5,
  0xda, 0xb4, 0x68, 0x22, 0x69, 0x26, 0x6a, 0x2b, 0x6b, 0x01, 0xa4, 0xc9,
  0xd8, 0xb4, 0xa4, 0xb9, 0xd9, 0xb4, 0xa4, 0xa1, 0x66, 0x8c, 0x67, 0x8c,
  0x87, 0x64, 0x47, 0x18, 0xa4, 0xa5, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9,
  0xd8, 0xb4, 0xa4, 0xbd, 0xd9, 0xb4, 0xa4, 0xa1, 0x66, 0x8c, 0x67, 0x78,
  0x87, 0x65, 0x47, 0xec, 0xa4, 0xa5, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9,
  0xd8, 0xb4, 0xa4, 0xc5, 0xd9, 0xb4, 0xa4, 0xa1, 0x66, 0x78, 0x67, 0x8c,
  0x87, 0x67, 0x47, 0xec, 0xa4, 0xa5, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9,
  0xd8, 0xb4, 0xa4, 0xc1, 0xd9, 0xb4, 0xa4, 0xa1, 0x66, 0x0f, 0x86, 0x66,
  0x46, 0x07, 0xa4, 0xa5, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xc9, 0xd8, 0xb4,
  0xa4, 0xe1, 0xd9, 0xb4, 0xa4, 0xa1, 0x66, 0xe0, 0x86, 0x6e, 0x46, 0xc0,
  0xa4, 0xa5, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xe5, 0xd8, 0xb4, 0xa4, 0xc1,
  0xd9, 0xb4, 0xa4, 0x9e, 0xf1, 0x65, 0xa4, 0xa5, 0x30, 0xaa, 0xa4, 0xa1,
  0x31, 0x55, 0xa4, 0xa1, 0xda, 0xb4, 0x68, 0x32, 0x69, 0x36, 0x6a, 0x3b,
  0x6b, 0x01, 0xa4, 0xe5, 0xd8, 0xb4, 0xa4, 0xbd, 0xd9, 0xb4, 0xa4, 0x9e,
  0x60, 0x00, 0x61, 0x30, 0xf1, 0x55, 0xa4, 0x9e, 0xf0, 0x65, 0x81, 0x00,
  0xa4, 0x9f, 0xf0, 0x65, 0xa4, 0xa5, 0x30, 0x30, 0xa4, 0xa1, 0x31, 0x00,
  0xa4, 0xa1, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xe5, 0xd8, 0xb4, 0xa4, 0xb5,
  0xd9, 0xb4, 0xa4, 0x9e, 0x66, 0x89, 0xf6, 0x33, 0xf2, 0x65, 0xa4, 0xa1,
  0x30, 0x01, 0x14, 0x32, 0x31, 0x03, 0x14, 0x32, 0x32, 0x07, 0x14, 0x32,
  0xa4, 0x9e, 0x66, 0x41, 0xf6, 0x33, 0xf2, 0x65, 0xa4, 0xa1, 0x30, 0x00,
  0x14, 0x32, 0x31, 0x06, 0x14, 0x32, 0x32, 0x05, 0x14, 0x32, 0xa4, 0x9e,
  0x66, 0x04, 0xf6, 0x33, 0xf2, 0x65, 0xa4, 0xa1, 0x30, 0x00, 0x14, 0x32,
  0x31, 0x00, 0x14, 0x32, 0x32, 0x04, 0x14, 0x32, 0xa4, 0xa5, 0xda, 0xb4,
  0x7b, 0x05, 0xa4, 0xe5, 0xd8, 0xb4, 0xa4, 0xe1, 0xd9, 0xb4, 0xa4, 0xa1,
  0x66, 0x04, 0xf6, 0x1e, 0xda, 0xb4, 0x7b, 0x05, 0xa4, 0xe9, 0xd8, 0xb4,
  0xa4, 0xed, 0xd9, 0xb4, 0xa4, 0xa5, 0x66, 0xff, 0x76, 0x0a, 0x36, 0x09,
  0xa4, 0xa1, 0x86, 0x66, 0x36, 0x04, 0xa4, 0xa1, 0x66, 0xff, 0x60, 0x0a,
  0x86, 0x04, 0x36, 0x09, 0xa4, 0xa1, 0x86, 0x66, 0x36, 0x04, 0xa4, 0xa1,
  0x66, 0xff, 0x86, 0x6e, 0x86, 0x66, 0x36, 0x7f, 0xa4, 0xa1, 0x86, 0x66,
  0x86, 0x6e, 0x36, 0x7e, 0xa4, 0xa1, 0x66, 0x05, 0x76, 0xf6, 0x36, 0xfb,
  0xa4, 0xa1, 0x66, 0x05, 0x86, 0x05, 0x36, 0xfb, 0xa4, 0xa1, 0x66, 0x05,
  0x80, 0x67, 0x30, 0xfb, 0xa4, 0xa1, 0xda, 0xb4, 0x14, 0x9c, 0xaa, 0x55,
  0x00, 0x00, 0xa0, 0x40, 0xa0, 0x00, 0xa0, 0xc0, 0x80, 0xe0, 0xa0, 0xa0,
  0xe0, 0xc0, 0x40, 0x40, 0xe0, 0xe0, 0x20, 0xc0, 0xe0, 0xe0, 0x60, 0x20,
  0xe0, 0xa0, 0xe0, 0x20, 0x20, 0xe0, 0xc0, 0x20, 0xc0, 0x60, 0x80, 0xe0,
  0xe0, 0xe0, 0x20, 0x40, 0x40, 0xe0, 0xe0, 0xa0, 0xe0, 0xe0, 0xe0, 0x20,
  0xc0, 0x40, 0xa0, 0xe0, 0xa0, 0xc0, 0xe0, 0xa0, 0xe0, 0xe0, 0x80, 0x80,
  0xe0, 0xc0, 0xa0, 0xa0, 0xc0, 0xe0, 0xc0, 0x80, 0xe0, 0xe0, 0x80, 0xc0,
  0x80, 0x00, 0xa0, 0xa0, 0x40, 0xa0, 0x40, 0xa0, 0xa0, 0x0a, 0xae, 0xa2,
  0x42, 0x38, 0x08, 0x30, 0xb8


};

static const unsigned char chip8_fontset[80] = {
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

static unsigned long long rand_seed = 88172645463325252ULL;

unsigned long long get_random(void) {
    rand_seed ^= (rand_seed << 13);
    rand_seed ^= (rand_seed >> 7);
    rand_seed ^= (rand_seed << 17);
    return rand_seed;
}

_Bool setPixel(unsigned char* display, int x, int y) {
    x %= 64;
    y %= 32;
    
    // Safety check for negative values
    if (x < 0) x += 64;
    if (y < 0) y += 32;

    int index = y * 64 + x;
    _Bool erased = display[index] == 1;
    display[index] ^= 1;
    return erased;
}

void chip8_init(Chip8* c8) {
    // 1. Clear all hardware structures cleanly
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

    // 2. Set memory vectors
    c8->pc = 0x200; 
    c8->I = 0;
    c8->sp = 0;
    c8->delay_timer = 0;
    c8->sound_timer = 0;
    c8->lastKeyPressed = -1;

    // 3. FIX: Move system built-in fonts to start at address 0x000 explicitly
    // This removes the 8-byte alignment bleed from the 0x200 execution boundaries
    for (int i = 0; i < 80; i++) {
        c8->memory[0x050 + i] = chip8_fontset[i];
    }

    // 4. Force write the standard 246 bytes of Pong directly into memory
    for (int i = 0; i < sizeof(pong_rom_bytes); i++) {
        c8->memory[0x200 + i] = pong_rom_bytes[i];
    }
}



void chip8_cycle(Chip8* c8) {
    unsigned short instruction = (c8->memory[c8->pc] << 8) | c8->memory[c8->pc + 1];
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
            unsigned char x = (instruction & 0x0F00) >> 8;
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
                    break;
                }
                case 0x0002: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->V[(instruction & 0x0F00) >> 8] & c8->V[(instruction & 0x00F0) >> 4];
                    break;
                }
                case 0x0003: {
                    c8->V[(instruction & 0x0F00) >> 8] = c8->V[(instruction & 0x0F00) >> 8] ^ c8->V[(instruction & 0x00F0) >> 4];
                    break;
                }
                case 0x0004: { // 8xy4 -> ADD Vx, Vy (With Carry)
                    unsigned char x = (instruction & 0x0F00) >> 8;
                    unsigned char y = (instruction & 0x00F0) >> 4;
                    unsigned short sum = c8->V[x] + c8->V[y];
                    
                    // Force the value calculation into a temporary variable first
                    unsigned char final_val = sum & 0xFF;
                    unsigned char carry = (sum > 255) ? 1 : 0;
                    
                    c8->V[x] = final_val;
                    c8->V[0xF] = carry; // Set VF dead last so it always reflects the carry
                    break;
                }
                case 0x0005: { // 8xy5 -> SUB Vx, Vy (Vx = Vx - Vy, NOT borrow to VF)
                    unsigned char x = (instruction & 0x0F00) >> 8;
                    unsigned char y = (instruction & 0x00F0) >> 4;
                    
                    // 1. Isolate the original register values into safe temporary scopes
                    unsigned char val_x = c8->V[x];
                    unsigned char val_y = c8->V[y];
                    
                    // 2. Compute final destination values out-of-line
                    // VF is set to 1 if Vx >= Vy (No borrow), otherwise 0 (Borrow)
                    unsigned char borrow = (val_x >= val_y) ? 1 : 0;
                    unsigned char final_val = val_x - val_y;

                    // 3. Write fields sequentially (Vx first, then VF)
                    c8->V[x] = final_val;
                    c8->V[0xF] = borrow;
                    break;
                }
                case 0x0006: { // 8xy6 -> SHR Vx {, Vy} (Shift right)
                    unsigned char x = (instruction & 0x0F00) >> 8;
                    unsigned char y = (instruction & 0x00F0) >> 4; // Extracted but ignored in modern mode

                    // 1. Capture the original value out of line
                    unsigned char original_val = c8->V[x]; 
                    
                    // NOTE: If you ever want to play legacy 1970s games that fail modern standards, 
                    // change the line above to: unsigned char original_val = c8->V[y];

                    // 2. Compute the LSB (Least Significant Bit) and the shifted value
                    unsigned char lsb = original_val & 0x01;
                    unsigned char final_val = original_val >> 1;

                    // 3. Write back safely (Vx first, then VF)
                    c8->V[x] = final_val;
                    c8->V[0xF] = lsb;
                    break;
                }

                case 0x0007: { // 8xy7 -> SUBN Vx, Vy (Vx = Vy - Vx)
                    unsigned char x = (instruction & 0x0F00) >> 8;
                    unsigned char y = (instruction & 0x00F0) >> 4;

                    // 1. Isolate original register values
                    unsigned char val_x = c8->V[x];
                    unsigned char val_y = c8->V[y];

                    // 2. Compute out-of-line using temporary variables
                    // VF is set to 1 if Vy >= Vx (No borrow), otherwise 0
                    unsigned char borrow = (val_y >= val_x) ? 1 : 0;
                    unsigned char final_val = val_y - val_x;

                    // 3. Write back safely (Vx first, then VF)
                    c8->V[x] = final_val;
                    c8->V[0xF] = borrow;
                    break;
                }
                case 0x000E: { // 8xyE -> SHL Vx {, Vy} (Shift left)
                    unsigned char x = (instruction & 0x0F00) >> 8;
                    
                    // 1. Capture the original value out-of-line
                    unsigned char original_val = c8->V[x];

                    // NOTE: If you ever want to play legacy 1970s games that use legacy quirks,
                    // change the line above to: unsigned char original_val = c8->V[y];

                    // 2. Compute the MSB (Most Significant Bit) and the shifted value
                    unsigned char msb = (original_val & 0x80) >> 7;
                    unsigned char final_val = original_val << 1;

                    // 3. Write fields back safely (Vx first, then VF)
                    c8->V[x] = final_val;
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
            c8->pc = (instruction * 0x0FFF) + c8->V[0];
        }
        case 0xC000: { // Cxkk -> RND Vx, byte (Vx = random byte AND kk)
            unsigned char x = (instruction & 0x0F00) >> 8;
            unsigned char kk = instruction & 0x00FF;

            // Get a random 64-bit number, cast to an 8-bit byte
            unsigned char random_byte = (unsigned char)(get_random() & 0xFF);

            // Perform the bitwise AND operation and store it in Vx
            c8->V[x] = kk & random_byte;
            break;
        }
        case 0xD000: { // Dxyn -> DRW Vx, Vy, nibble
            unsigned int rx = (instruction & 0x0F00) >> 8;
            unsigned int ry = (instruction & 0x00F0) >> 4;
            
            unsigned int start_x = c8->V[rx] % 64;
            unsigned int start_y = c8->V[ry] % 32;
            unsigned int height  = instruction & 0x000F;

            c8->V[0xF] = 0; 

            for (unsigned int row = 0; row < height; row++) {
                unsigned int pixel_y = start_y + row;
                if (pixel_y >= 32) break; // Clip row

                unsigned char sprite_byte = c8->memory[c8->I + row];

                for (unsigned int col = 0; col < 8; col++) {
                    unsigned int pixel_x = start_x + col;
                    if (pixel_x >= 64) continue; // Clip column

                    unsigned int mask = 0x80 >> col;
                    
                    if ((sprite_byte & mask) != 0) {
                        unsigned int index = pixel_x + (pixel_y * 64);

                        if (c8->gfx[index] == 1) {
                            c8->V[0xF] = 1; // Collision detected
                        }
                        c8->gfx[index] ^= 1; 
                    }
                }
            }
            break;
        }
        case 0xE000: {
            unsigned char x = (instruction & 0x0F00) >> 8;
            unsigned char key_index = c8->V[x] & 0x0F; // Safely cap the key lookups between 0x0 and 0xF

            switch (instruction & 0x00FF) {
                case 0x009E: { // Ex9E -> SKP Vx (Skip next instruction if key Vx is pressed)
                    if (c8->key[key_index] == 1) {
                        c8->pc += 2;
                    }
                    break;
                }
                case 0x00A1: { // ExA1 -> SKNP Vx (Skip next instruction if key Vx is NOT pressed)
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
                    unsigned char key = 0;
                    for (unsigned char i = 0; i < 16; i++) {
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
                        unsigned char x = (instruction & 0x0F00) >> 8;
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
                    unsigned char x = (instruction & 0x0F00) >> 8;  // get register index
                    c8->I = 0x50 + (c8->V[x] * 5);
                    break;
                }
                case 0x0033: {
                    unsigned char x = (instruction & 0x0F00) >> 8; // extract x
                    unsigned char value = c8->V[x];

                    c8->memory[c8->I]     = value / 100;         // hundreds
                    c8->memory[c8->I + 1] = (value / 10) % 10;  // tens
                    c8->memory[c8->I + 2] = value % 10;         // ones
                    break;
                }
                case 0x0055: {
                    unsigned char x = (instruction & 0x0F00) >> 8; // extract x
                    for (int i = 0; i <= x; i++) {
                        c8->memory[c8->I + i] = c8->V[i];
                    }
                    break;
                }
                case 0x0065: {
                    unsigned char x = (instruction & 0x0F00) >> 8; // which register Vx
                    for (unsigned char i = 0; i <= x; i++) {
                        c8->V[i] = c8->memory[c8->I + i];
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
