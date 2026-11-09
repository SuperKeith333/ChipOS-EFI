#ifndef CHIP_SCREEN_H
#define CHIP_SCREEN_H

#include "screen.h"
#include "emulator/cpu.h"
#include "renderer.h"

const uint16_t OPCODES_PER_FRAME = 10;

typedef struct {
    Chip8* c8;
    uint8_t rom[3584];
    uint16_t romSize;
} c8_start;

void chipScreen_start(screen* _screen, void* input) {
    c8_start* _c8_start = (c8_start*)input;
    chip8_init(_c8_start->c8, _c8_start->rom, _c8_start->romSize);
}

void chipScreen_input(screen* _screen, void* input) {
    Chip8* c8 = (Chip8*)input;
    uint8_t live_scancode = kernel_get_scancode();
    if (live_scancode == 0x81) {
        kernel_stop_sound();
        _screen->swap(FILE_SCREEN);
    }

    switch (live_scancode) {
        // Row 1: 1, 2, 3, 4  ->  1, 2, 3, C
        case 0x02: c8->key[0x1] = 1; break; // '1' pressed
        case 0x82: c8->key[0x1] = 0; break; // '1' released
        case 0x03: c8->key[0x2] = 1; break; // '2' pressed
        case 0x83: c8->key[0x2] = 0; break; // '2' released
        case 0x04: c8->key[0x3] = 1; break; // '3' pressed
        case 0x84: c8->key[0x3] = 0; break; // '3' released
        case 0x05: c8->key[0xC] = 1; break; // '4' pressed
        case 0x85: c8->key[0xC] = 0; break; // '4' released

            // Row 2: Q, W, E, R  ->  4, 5, 6, D
        case 0x10: c8->key[0x4] = 1; break; // 'Q' pressed
        case 0x90: c8->key[0x4] = 0; break; // 'Q' released
        case 0x11: c8->key[0x5] = 1; break; // 'W' pressed
        case 0x91: c8->key[0x5] = 0; break; // 'W' released
        case 0x12: c8->key[0x6] = 1; break; // 'E' pressed
        case 0x92: c8->key[0x6] = 0; break; // 'E' released
        case 0x13: c8->key[0xD] = 1; break; // 'R' pressed
        case 0x93: c8->key[0xD] = 0; break; // 'R' released

            // Row 3: A, S, D, F  ->  7, 8, 9, E
        case 0x1E: c8->key[0x7] = 1; break; // 'A' pressed
        case 0x9E: c8->key[0x7] = 0; break; // 'A' released
        case 0x1F: c8->key[0x8] = 1; break; // 'S' pressed
        case 0x9F: c8->key[0x8] = 0; break; // 'S' released
        case 0x20: c8->key[0x9] = 1; break; // 'D' pressed
        case 0xA0: c8->key[0x9] = 0; break; // 'D' released
        case 0x21: c8->key[0xE] = 1; break; // 'F' pressed
        case 0xA1: c8->key[0xE] = 0; break; // 'F' released

            // Row 4: Z, X, C, V  ->  A, 0, B, F
        case 0x2C: c8->key[0xA] = 1; break; // 'Z' pressed
        case 0xAC: c8->key[0xA] = 0; break; // 'Z' released
        case 0x2D: c8->key[0x0] = 1; break; // 'X' pressed
        case 0xAD: c8->key[0x0] = 0; break; // 'X' released
        case 0x2E: c8->key[0xB] = 1; break; // 'C' pressed
        case 0xAE: c8->key[0xB] = 0; break; // 'C' released
        case 0x2F: c8->key[0xF] = 1; break; // 'V' pressed
        case 0xAF: c8->key[0xF] = 0; break; // 'V' released

        default: break;
    }
}

void chipScreen_tick(screen* _screen, void* input) {
    Chip8* c8 = (Chip8*)input;

    chip8_update_timers(c8);

    for (uint16_t i = 0; i < OPCODES_PER_FRAME; i++) {
        if (!c8->drewSprite)
            chip8_cycle(c8);
    }

    if (c8->sound_timer > 0) {
        kernel_play_sound(440);
    } else {
        kernel_stop_sound();
    }

    c8->drewSprite = 0;
}

void chipScreen_render(screen* _screen, void* input) {
    Chip8* c8 = (Chip8*)input;
    uint16_t panel_x = 900;

    // Game Monitor Panel
    cursor_x = 450; cursor_y = 35;
    kernel_print(_screen->binfo, "VIRTUAL GAME MONITOR\n", 0x0000FFFF);
    kernel_draw_box(_screen->binfo, 200, 60, 642, 322, 0x00FFFFFF);
    kernel_draw_chip8_screen(_screen->binfo, c8->gfx, 201, 61, 10, 0x0000FF00, 0x00080812);

    uint16_t current_opcode = (c8->memory[c8->pc] << 8) | c8->memory[c8->pc + 1];

    // Hardware Core CPU Debugger Panel
    cursor_x = panel_x; cursor_y = 35;
    kernel_print(_screen->binfo, "CPU CORE REGISTERS & ARCHITECTURE STATUS\n", 0x00FF00FF);
    kernel_draw_box(_screen->binfo, panel_x, 60, 310, 480, 0x00505070);

    cursor_x = panel_x + 15; cursor_y = 80;
    kernel_print(_screen->binfo, "PC STATUS : ", 0x00A0A0A0);
    kernel_print_hex16(_screen->binfo, c8->pc, 0x00FFFF00);

    cursor_x = panel_x + 15; cursor_y = 100;
    kernel_print(_screen->binfo, "OPCODE    : ", 0x00A0A0A0);
    kernel_print_hex16(_screen->binfo, current_opcode, 0x0000FFFF); 

    cursor_x = panel_x + 15; cursor_y = 120;
    kernel_print(_screen->binfo, "INDEX (I) : ", 0x00A0A0A0);
    kernel_print_hex16(_screen->binfo, c8->I, 0x00FFFF00);

    cursor_x = panel_x + 15; cursor_y = 140;
    kernel_print(_screen->binfo, "TIMERS    : DT:", 0x00A0A0A0);
    kernel_print_hex8(_screen->binfo, c8->delay_timer, 0x0000FF00);
    kernel_print(_screen->binfo, " ST:", 0x00A0A0A0);
    kernel_print_hex8(_screen->binfo, c8->sound_timer, 0x00FF3333);

    cursor_x = panel_x + 15; cursor_y = 160;
    kernel_print(_screen->binfo, "STACK PTR : SP:", 0x00A0A0A0);
    kernel_print_int(_screen->binfo, c8->sp, 0x0000FFFF);
    kernel_print(_screen->binfo, " TOP:", 0x00A0A0A0);
    kernel_print_hex16(_screen->binfo, c8->stack[c8->sp > 0 ? c8->sp - 1 : 0], 0x0000FFFF);

    cursor_x = panel_x + 15; cursor_y = 195;
    kernel_print(_screen->binfo, "--- REGISTER DATA MATRIX ---\n", 0x0050FF50);
    for (int i = 0; i < 16; i++) {
        if (i < 8) {
            cursor_x = panel_x + 15;
            cursor_y = 220 + (i * 20);
        } else {
            cursor_x = panel_x + 165;
            cursor_y = 220 + ((i - 8) * 20);
        }
        uint8_t reg_label[] = "V_ : ";
        reg_label[1] = (i < 10) ? (i + '0') : (i - 10 + 'A');
            
        kernel_print(_screen->binfo, (char*)reg_label, 0x00FFFFFF);
        kernel_print_hex8(_screen->binfo, c8->V[i], 0x00FFFF80);
    }

}
#endif