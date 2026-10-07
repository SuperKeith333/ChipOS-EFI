#ifndef RENDERER_H
#define RENDERER_H

#include "bootinfo.h"

// Expose global cursor variables
extern unsigned int cursor_x;
extern unsigned int cursor_y;

// Function prototypes
void kernel_blit(unsigned int* dest, unsigned int* src, unsigned long long pixel_count);
void kernel_putchar(BootInfo* binfo, char c, unsigned int color);
void kernel_print(BootInfo* binfo, const char* str, unsigned int color);
void kernel_print_int(BootInfo* binfo, unsigned long long num, unsigned int color);
void kernel_clear_screen(BootInfo* binfo, unsigned int color);
void kernel_draw_box(BootInfo* binfo, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color);
void kernel_print_hex(BootInfo* binfo, unsigned long long value, unsigned int color);
void kernel_draw_chip8_screen(BootInfo* binfo, unsigned char* gfx, unsigned int start_x, unsigned int start_y, unsigned int scale, unsigned int pixel_color, unsigned int bg_color);
void kernel_print_hex8(BootInfo* binfo, unsigned char value, unsigned int color);
void kernel_print_hex16(BootInfo* binfo, unsigned short value, unsigned int color);
unsigned char kernel_get_scancode(void);
void kernel_play_sound(unsigned int frequency);
void kernel_stop_sound(void);


#endif
