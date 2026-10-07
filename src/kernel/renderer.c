#include "renderer.h"

// Access font binary embedded by objcopy
extern unsigned char _binary_font_bin_start[];

// Instantiating the global trackers
unsigned int cursor_x = 50;
unsigned int cursor_y = 50;

void kernel_blit(unsigned int* dest, unsigned int* src, unsigned long long pixel_count) {
    for (unsigned long long i = 0; i < pixel_count; i++) {
        dest[i] = src[i];
    }
}

void kernel_clear_screen(BootInfo* binfo, unsigned int color) {
    unsigned long long total_pixels = binfo->Height * binfo->PixelsPerScanLine;
    for (unsigned long long i = 0; i < total_pixels; i++) {
        binfo->BackBuffer[i] = color;
    }
}

void kernel_putchar(BootInfo* binfo, char c, unsigned int color) {
    unsigned char character_index = (unsigned char)c;
    unsigned char* glyph = &_binary_font_bin_start[character_index * 16];

    for (int y = 0; y < 16; y++) {
        unsigned char font_row = glyph[y];
        for (int x = 0; x < 8; x++) {
            if ((font_row << x) & 0x80) {
                unsigned long long offset = (cursor_x + x) + ((cursor_y + y) * binfo->PixelsPerScanLine);
                binfo->BackBuffer[offset] = color;
            }
        }
    }
    cursor_x += 8;
}

void kernel_print(BootInfo* binfo, const char* str, unsigned int color) {
    while (*str) {
        if (*str == '\n') {
            cursor_x = 50;
            cursor_y += 20;
        } else {
            kernel_putchar(binfo, *str, color);
        }
        str++;
    }
}

void kernel_print_int(BootInfo* binfo, unsigned long long num, unsigned int color) {
    char buffer[21];
    int i = 19;
    buffer[20] = '\0';
    
    if (num == 0) {
        buffer[i--] = '0';
    } else {
        while (num > 0 && i >= 0) {
            buffer[i--] = (num % 10) + '0';
            num /= 10;
        }
    }
    kernel_print(binfo, &buffer[i + 1], color);
}

void kernel_draw_box(BootInfo* binfo, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color) {
    // Top and Bottom horizontal lines
    for (unsigned int i = 0; i < width; i++) {
        if ((x + i) < binfo->Width) {
            if (y < binfo->Height) 
                binfo->BackBuffer[(x + i) + (y * binfo->PixelsPerScanLine)] = color;
            if ((y + height - 1) < binfo->Height) 
                binfo->BackBuffer[(x + i) + ((y + height - 1) * binfo->PixelsPerScanLine)] = color;
        }
    }
    // Left and Right vertical lines
    for (unsigned int j = 0; j < height; j++) {
        if ((y + j) < binfo->Height) {
            if (x < binfo->Width) 
                binfo->BackBuffer[x + ((y + j) * binfo->PixelsPerScanLine)] = color;
            if ((x + width - 1) < binfo->Width) 
                binfo->BackBuffer[(x + width - 1) + ((y + j) * binfo->PixelsPerScanLine)] = color;
        }
    }
}

// Prints an unsigned integer as a hexadecimal string (up to 64-bit values)
void kernel_print_hex(BootInfo* binfo, unsigned long long value, unsigned int color) {
    char hex_digits[] = "0123456789ABCDEF";
    char buffer[19]; // "0x" + 16 chars for 64-bit hex + null terminator
    
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = '\0';
    
    // Fill the buffer from right to left
    for (int i = 17; i >= 2; i--) {
        buffer[i] = hex_digits[value & 0xF];
        value >>= 4;
    }
    
    kernel_print(binfo, buffer, color);
}

// Draws the 64x32 Chip-8 graphics matrix onto your OS backbuffer with an adjustable pixel scale factor
void kernel_draw_chip8_screen(BootInfo* binfo, unsigned char* gfx, unsigned int start_x, unsigned int start_y, unsigned int scale, unsigned int pixel_color, unsigned int bg_color) {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            // Read standard 1D flat chip8 array state (64 width step size)
            unsigned char pixel_state = gfx[x + (y * 64)];
            unsigned int current_color = pixel_state ? pixel_color : bg_color;

            // DRAW SCALED BLOCK
            // Explicitly map the chip8 coordinates up to host sizes out-of-line
            unsigned int block_start_x = start_x + (x * scale);
            unsigned int block_start_y = start_y + (y * scale);

            for (unsigned int sy = 0; sy < scale; sy++) {
                unsigned int dest_y = block_start_y + sy;
                
                // Keep performance clipped to monitor bounding layout boxes
                if (dest_y >= binfo->Height) continue;

                // Precompute row index using the motherboard's actual Hardware Scan Line Stride!
                unsigned long long row_offset = (unsigned long long)dest_y * binfo->PixelsPerScanLine;

                for (unsigned int sx = 0; sx < scale; sx++) {
                    unsigned int dest_x = block_start_x + sx;

                    if (dest_x < binfo->Width) {
                        // FIX: Linear 32-bit pixel array alignment calculation
                        binfo->BackBuffer[dest_x + row_offset] = current_color;
                    }
                }
            }
        }
    }
}



// Prints an 8-bit unsigned value as a clean 2-digit hex string (e.g., 0xAB)
void kernel_print_hex8(BootInfo* binfo, unsigned char value, unsigned int color) {
    char hex_digits[] = "0123456789ABCDEF";
    char buffer[5]; // "0x" + 2 digits + null terminator
    
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = hex_digits[(value >> 4) & 0x0F]; // Upper nibble
    buffer[3] = hex_digits[value & 0x0F];        // Lower nibble
    buffer[4] = '\0';
    
    kernel_print(binfo, buffer, color);
}

// Prints a 16-bit unsigned value as a clean 4-digit hex string (e.g., 0x02D4)
void kernel_print_hex16(BootInfo* binfo, unsigned short value, unsigned int color) {
    char hex_digits[] = "0123456789ABCDEF";
    char buffer[7]; // "0x" + 4 digits + null terminator
    
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = hex_digits[(value >> 12) & 0x0F];
    buffer[3] = hex_digits[(value >> 8) & 0x0F];
    buffer[4] = hex_digits[(value >> 4) & 0x0F];
    buffer[5] = hex_digits[value & 0x0F];
    buffer[6] = '\0';
    
    kernel_print(binfo, buffer, color);
}

// Low-level assembly function to read a byte from an I/O port
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Non-blocking function: Returns the raw scancode if a key event is waiting, 0 otherwise
unsigned char kernel_get_scancode(void) {
    // Check Port 0x64 (Status Register). Bit 0 is set (1) if data is ready in the input buffer.
    if (inb(0x64) & 0x01) {
        return inb(0x60); // Read the scancode from Port 0x60
    }
    return 0;
}

// Low-level port out function (if not already defined in your files)
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

// Low-level port in function (if not already defined in your files)
static inline unsigned char inb_speaker(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Play a tone at a specific frequency (e.g., 440 Hz)
void kernel_play_sound(unsigned int frequency) {
    unsigned int div;
    unsigned char tmp;

    // Calculate the frequency divider for the 1.193182 MHz PIT clock
    div = 1193180 / frequency;
    
    // Set the PIT to Channel 2, Square Wave Mode
    outb(0x43, 0xB6);
    outb(0x42, (unsigned char) (div & 0xFF));        // Low byte
    outb(0x42, (unsigned char) ((div >> 8) & 0xFF)); // High byte

    // Read the current PC speaker status port
    tmp = inb_speaker(0x61);
    
    // Set bits 0 and 1 to turn on PIT channel 2 output and connect it to the speaker
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

// Turn off the PC speaker entirely
void kernel_stop_sound(void) {
    unsigned char tmp = inb_speaker(0x61);
    outb(0x61, tmp & 0xFC); // Clear bits 0 and 1
}
