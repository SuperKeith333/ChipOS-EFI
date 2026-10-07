#include "bootinfo.h"
#include "renderer.h"
#include "emulator/cpu.h"

static inline unsigned long long rdtsc(void) {
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)hi << 32) | lo;
}

void kernel_main(BootInfo* binfo) {
    unsigned long long total_pixels = binfo->Height * binfo->PixelsPerScanLine;
    
    // Core Engine Clocks
    unsigned long long cycles_per_screen_frame = 16666 * binfo->CpuMhz; // 60 FPS Frame
    unsigned long long cycles_per_second = 1000000 * binfo->CpuMhz;

    unsigned long long last_screen_frame_time = rdtsc();
    unsigned long long last_fps_update = rdtsc();
    
    unsigned int screen_frame_counter = 0;
    unsigned int current_fps_display = 0;

    Chip8 c8;
    chip8_init(&c8);

    unsigned int panel_x = 900;

    // TARGET EMULATOR SPEED DEFINITION:
    // Running 10 instructions every 1/60th of a second frame yields exactly 600 Hz!
    const unsigned int OPCODES_PER_FRAME = 10; 

    _Bool runningChip8 = 0;

    while (1) {
        unsigned long long current_time = rdtsc();

        // Enforce the 60 Hz Frame Sync window
        if (current_time - last_screen_frame_time < cycles_per_screen_frame) {
            __asm__ volatile("pause");
            continue;
        }
        last_screen_frame_time = current_time;
        screen_frame_counter++;

        if (runningChip8) {
                    // --- 1. CAPTURE ACTIVE USER HOST INPUTS ---
            unsigned char live_scancode = kernel_get_scancode();

            // Comprehensive PS/2 Keyboard Scancode Matrix covering all 16 Chip-8 keys
            switch (live_scancode) {
                // Row 1: 1, 2, 3, 4  ->  1, 2, 3, C
                case 0x02: c8.key[0x1] = 1; break; // '1' pressed
                case 0x82: c8.key[0x1] = 0; break; // '1' released
                case 0x03: c8.key[0x2] = 1; break; // '2' pressed
                case 0x83: c8.key[0x2] = 0; break; // '2' released
                case 0x04: c8.key[0x3] = 1; break; // '3' pressed
                case 0x84: c8.key[0x3] = 0; break; // '3' released
                case 0x05: c8.key[0xC] = 1; break; // '4' pressed
                case 0x85: c8.key[0xC] = 0; break; // '4' released

                // Row 2: Q, W, E, R  ->  4, 5, 6, D
                case 0x10: c8.key[0x4] = 1; break; // 'Q' pressed
                case 0x90: c8.key[0x4] = 0; break; // 'Q' released
                case 0x11: c8.key[0x5] = 1; break; // 'W' pressed
                case 0x91: c8.key[0x5] = 0; break; // 'W' released
                case 0x12: c8.key[0x6] = 1; break; // 'E' pressed
                case 0x92: c8.key[0x6] = 0; break; // 'E' released
                case 0x13: c8.key[0xD] = 1; break; // 'R' pressed
                case 0x93: c8.key[0xD] = 0; break; // 'R' released

                // Row 3: A, S, D, F  ->  7, 8, 9, E
                case 0x1E: c8.key[0x7] = 1; break; // 'A' pressed
                case 0x9E: c8.key[0x7] = 0; break; // 'A' released
                case 0x1F: c8.key[0x8] = 1; break; // 'S' pressed
                case 0x9F: c8.key[0x8] = 0; break; // 'S' released
                case 0x20: c8.key[0x9] = 1; break; // 'D' pressed
                case 0xA0: c8.key[0x9] = 0; break; // 'D' released
                case 0x21: c8.key[0xE] = 1; break; // 'F' pressed
                case 0xA1: c8.key[0xE] = 0; break; // 'F' released

                // Row 4: Z, X, C, V  ->  A, 0, B, F
                case 0x2C: c8.key[0xA] = 1; break; // 'Z' pressed
                case 0xAC: c8.key[0xA] = 0; break; // 'Z' released
                case 0x2D: c8.key[0x0] = 1; break; // 'X' pressed
                case 0xAD: c8.key[0x0] = 0; break; // 'X' released
                case 0x2E: c8.key[0xB] = 1; break; // 'C' pressed
                case 0xAE: c8.key[0xB] = 0; break; // 'C' released
                case 0x2F: c8.key[0xF] = 1; break; // 'V' pressed
                case 0xAF: c8.key[0xF] = 0; break; // 'V' released

                default: break; // Pass through all other system keys safely
            }


            // ==================== FULL SPEED CORE EXECUTION ====================
            // 1. Run multiple Chip-8 instructions back-to-back this frame to match hardware speed
                    // ==================== FULL SPEED CORE EXECUTION ====================
            for (unsigned int i = 0; i < OPCODES_PER_FRAME; i++) {
                chip8_cycle(&c8);
            }

            chip8_update_timers(&c8);

            // FIX / ADDED: Check the Chip-8 sound timer state every frame at 60Hz
            if (c8.sound_timer > 0) {
                kernel_play_sound(440); // Play a standard 440Hz "beep" tone
            } else {
                kernel_stop_sound();    // Silence when timer hits zero
            }
            // ===================================================================

            // ===================================================================

            // Perform standard performance logging math updates
            if (current_time - last_fps_update >= cycles_per_second) {
                current_fps_display = screen_frame_counter;
                screen_frame_counter = 0;
                last_fps_update = current_time;
            }

            // --- GRAPHICS RENDERING BLOCK ---
            kernel_clear_screen(binfo, 0x0012121F);

            // PANE 1: Game Monitor Panel
            cursor_x = 450; cursor_y = 35;
            kernel_print(binfo, "VIRTUAL GAME MONITOR\n", 0x0000FFFF);
            kernel_draw_box(binfo, 200, 60, 642, 322, 0x00FFFFFF);
            kernel_draw_chip8_screen(binfo, c8.gfx, 201, 61, 10, 0x0000FF00, 0x00080812);

            unsigned short current_opcode = (c8.memory[c8.pc] << 8) | c8.memory[c8.pc + 1];

            // PANE 2: Hardware Core CPU Debugger Panel
            cursor_x = panel_x; cursor_y = 35;
            kernel_print(binfo, "CPU CORE REGISTERS & ARCHITECTURE STATUS\n", 0x00FF00FF);
            kernel_draw_box(binfo, panel_x, 60, 310, 480, 0x00505070);

            cursor_x = panel_x + 15; cursor_y = 80;
            kernel_print(binfo, "PC STATUS : ", 0x00A0A0A0);
            kernel_print_hex16(binfo, c8.pc, 0x00FFFF00);

            cursor_x = panel_x + 15; cursor_y = 100;
            kernel_print(binfo, "OPCODE    : ", 0x00A0A0A0);
            kernel_print_hex16(binfo, current_opcode, 0x0000FFFF); 

            cursor_x = panel_x + 15; cursor_y = 120;
            kernel_print(binfo, "INDEX (I) : ", 0x00A0A0A0);
            kernel_print_hex16(binfo, c8.I, 0x00FFFF00);

            cursor_x = panel_x + 15; cursor_y = 140;
            kernel_print(binfo, "TIMERS    : DT:", 0x00A0A0A0);
            kernel_print_hex8(binfo, c8.delay_timer, 0x0000FF00);
            kernel_print(binfo, " ST:", 0x00A0A0A0);
            kernel_print_hex8(binfo, c8.sound_timer, 0x00FF3333);

            cursor_x = panel_x + 15; cursor_y = 160;
            kernel_print(binfo, "STACK PTR : SP:", 0x00A0A0A0);
            kernel_print_int(binfo, c8.sp, 0x0000FFFF);
            kernel_print(binfo, " TOP:", 0x00A0A0A0);
            kernel_print_hex16(binfo, c8.stack[c8.sp > 0 ? c8.sp - 1 : 0], 0x0000FFFF);

            cursor_x = panel_x + 15; cursor_y = 195;
            kernel_print(binfo, "--- REGISTER DATA MATRIX ---\n", 0x0050FF50);

            for (int i = 0; i < 16; i++) {
                if (i < 8) {
                    cursor_x = panel_x + 15;
                    cursor_y = 220 + (i * 20);
                } else {
                    cursor_x = panel_x + 165;
                    cursor_y = 220 + ((i - 8) * 20);
                }

                char reg_label[] = "V_ : ";
                // FIX: Target index 1 to replace the underscore character with your Hex string label
                reg_label[1] = (i < 10) ? (i + '0') : (i - 10 + 'A');
                
                kernel_print(binfo, reg_label, 0x00FFFFFF);
                kernel_print_hex8(binfo, c8.V[i], 0x00FFFF80); // Ensure c8-> pointer syntax is used
            }
        } else {
            unsigned char live_scancode = kernel_get_scancode();

            // Enter is Pressed
            if (live_scancode == 0x1C) {
                runningChip8 = 1;
            }

            kernel_clear_screen(binfo, 0x0012121F);

            // Draw Window Outline
            kernel_draw_box(binfo, 60, 60, 1150, 640, 0x0050FF50);
            cursor_x = 550; cursor_y = 45;
            kernel_print(binfo, "--- CHOOSE A ROM TO RUN ---\n", 0x00FF00FF);
        }


        

        kernel_blit(binfo->Framebuffer, binfo->BackBuffer, total_pixels);
    }
}
