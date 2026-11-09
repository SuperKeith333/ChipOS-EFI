#include <stdint.h>

#include "bootinfo.h"
#include "renderer.h"
#include "emulator/cpu.h"
#include "physical_memory_manager.h"
#include "heap_allocator.h"
#include "pci.h"
#include "drivers/storage/ahci.h"
#include "drivers/storage/block_device.h"
#include "drivers/storage/diskio_adapter.h"
#include "io.h"
#include "helpers.h"
#include "drivers/storage/sdhci.h"
#include "ff.h"
#include "error_screen.h"
#include "chip_screen.h"
#include "file_screen.h"

#include <string.h>

physicalMemoryManager memoryManager;
heap _heap;
pciDeviceInfo deviceInfo;
BlockDevice blockDevice;
Chip8 c8;
FATFS fatFs;
DIR rootDir;
FILINFO fileInfo;

_Bool runningChip8 = 0;
_Bool error = 0;
errorType _errorType = NONE;
int errorCode = 0;
FRESULT result = 0;
int fileCount = 0;
char fileNames[20];
screen errorScreen;
activeScreen _activeScreen;
activeScreen lastActiveScreen;
errorHeader _errorHeader;
screen chipScreen;
screen fileScreen;
fileData _fileData;
c8_start _c8_start;

uint32_t* _16BitAddress = 0;
uint32_t* _32BitAddress = 0;
uint32_t* _64BitAddress = 0;
uint32_t* _128BitAddress = 0;
uint32_t* _256BitAddress = 0;
uint32_t* _512BitAddress = 0;
uint64_t readFileAddr;
int ahciInitialized = 0;
int framesPassed = 0;

uint8_t baseClass;
uint8_t subClass;

void set_screen(activeScreen screen) {
    _activeScreen = screen;
}

void start() {
    // chip8_init(&c8);
}

void draw(BootInfo* binfo) {
    switch (_activeScreen) {
        case ERROR_SCREEN:
            errorScreen.render(&errorScreen, (void*)&_errorHeader);
            break;
        case CHIP_SCREEN:
            chipScreen.render(&chipScreen, (void*)&c8);
            break;
        case FILE_SCREEN:
            fileScreen.render(&fileScreen, (void*)&_fileData);
            break;
    }
    // if (error) {
    //     kernel_draw_box(binfo, 200, 60, 875, 200, 0x00FFFFFF);

    //     switch (_errorType)
    //     {
    //         case BLOCK_DEVICE_ERROR:
    //             cursor_x = 600; cursor_y = 70;
    //             kernel_print(binfo, "BLOCK_DEVICE_ERROR", 0x0000FFFF);
    //             switch (errorCode)
    //             {
    //                 case 1:
    //                     cursor_x = 580; cursor_y = 100;
    //                     kernel_print(binfo, "BLOCK_DEVICE_DEVICE_ERROR", 0x0000FFFF);
    //                     break;
    //                 case 2:
    //                     cursor_x = 560; cursor_y = 100;
    //                     kernel_print(binfo, "BLOCK_DEVICE_DEVICE_NOT_READY", 0x0000FFFF);
    //                     break;
    //                 case 3:
    //                     cursor_x = 580; cursor_y = 100;
    //                     kernel_print(binfo, "BLOCK_DEVICE_INVALID_LBA", 0x0000FFFF);
    //                     break;
    //                 case 4:
    //                     cursor_x = 560; cursor_y = 100;
    //                     kernel_print(binfo, "BLOCK_DEVICE_INVALID_ARGUMENT", 0x0000FFFF);
    //                     break;
    //                 case 5:
    //                     cursor_x = 595; cursor_y = 100;
    //                     kernel_print(binfo, "BLOCK_DEVICE_TIMEOUT", 0x0000FFFF);
    //                     break;
    //                 case 6:
    //                     cursor_x = 590; cursor_y = 100;
    //                     kernel_print(binfo, "BLOCK_DEVICE_IO_ERROR", 0x0000FFFF);
    //                     break;
                    
    //                 default:
    //                     break;
    //             }
    //             break;
    //         case SDHCI_ERROR:
    //             cursor_x = 620; cursor_y = 70;
    //             kernel_print(binfo, "SDHCI_ERROR", 0x0000FFFF);
    //             switch (errorCode)
    //             {
    //                 case 1:
    //                     cursor_x = 560; cursor_y = 100;
    //                     kernel_print(binfo, "SDHCI_SOFTWARE_RESET_TIMEOUT", 0x0000FFFF);
    //                     break;
    //                 case 2:
    //                     cursor_x = 520; cursor_y = 100;
    //                     kernel_print(binfo, "SDHCI_INTERNAL_CLOCK_STABILITY_TIMEOUT", 0x0000FFFF);
    //                     break;
    //                 case 3:
    //                     cursor_x = 520; cursor_y = 100;
    //                     kernel_print(binfo, "SDHCI_FAILED_TO_ALLOCATE_HEAP_MEMORY", 0x0000FFFF);
    //                     break;
    //                 case 4:
    //                     cursor_x = 580; cursor_y = 100;
    //                     kernel_print(binfo, "SDHCI_GENERIC_TIMEOUT", 0x0000FFFF);
    //                     break;
                    
    //                 default:
    //                     break;
    //             }
    //             break;
    //         default:
    //             break;
    //     }
    // } else {
    //     if (runningChip8) {
    //         // Game Monitor Panel
    //         cursor_x = 450; cursor_y = 35;
    //         kernel_print(binfo, "VIRTUAL GAME MONITOR\n", 0x0000FFFF);
    //         kernel_draw_box(binfo, 200, 60, 642, 322, 0x00FFFFFF);
    //         kernel_draw_chip8_screen(binfo, c8.gfx, 201, 61, 10, 0x0000FF00, 0x00080812);

    //         uint16_t current_opcode = (c8.memory[c8.pc] << 8) | c8.memory[c8.pc + 1];

    //         // Hardware Core CPU Debugger Panel
    //         cursor_x = panel_x; cursor_y = 35;
    //         kernel_print(binfo, "CPU CORE REGISTERS & ARCHITECTURE STATUS\n", 0x00FF00FF);
    //         kernel_draw_box(binfo, panel_x, 60, 310, 480, 0x00505070);

    //         cursor_x = panel_x + 15; cursor_y = 80;
    //         kernel_print(binfo, "PC STATUS : ", 0x00A0A0A0);
    //         kernel_print_hex16(binfo, c8.pc, 0x00FFFF00);

    //         cursor_x = panel_x + 15; cursor_y = 100;
    //         kernel_print(binfo, "OPCODE    : ", 0x00A0A0A0);
    //         kernel_print_hex16(binfo, current_opcode, 0x0000FFFF); 

    //         cursor_x = panel_x + 15; cursor_y = 120;
    //         kernel_print(binfo, "INDEX (I) : ", 0x00A0A0A0);
    //         kernel_print_hex16(binfo, c8.I, 0x00FFFF00);

    //         cursor_x = panel_x + 15; cursor_y = 140;
    //         kernel_print(binfo, "TIMERS    : DT:", 0x00A0A0A0);
    //         kernel_print_hex8(binfo, c8.delay_timer, 0x0000FF00);
    //         kernel_print(binfo, " ST:", 0x00A0A0A0);
    //         kernel_print_hex8(binfo, c8.sound_timer, 0x00FF3333);

    //         cursor_x = panel_x + 15; cursor_y = 160;
    //         kernel_print(binfo, "STACK PTR : SP:", 0x00A0A0A0);
    //         kernel_print_int(binfo, c8.sp, 0x0000FFFF);
    //         kernel_print(binfo, " TOP:", 0x00A0A0A0);
    //         kernel_print_hex16(binfo, c8.stack[c8.sp > 0 ? c8.sp - 1 : 0], 0x0000FFFF);

    //         cursor_x = panel_x + 15; cursor_y = 195;
    //         kernel_print(binfo, "--- REGISTER DATA MATRIX ---\n", 0x0050FF50);

    //         for (int i = 0; i < 16; i++) {
    //             if (i < 8) {
    //                 cursor_x = panel_x + 15;
    //                 cursor_y = 220 + (i * 20);
    //             } else {
    //                 cursor_x = panel_x + 165;
    //                 cursor_y = 220 + ((i - 8) * 20);
    //             }

    //             uint8_t reg_label[] = "V_ : ";
    //             reg_label[1] = (i < 10) ? (i + '0') : (i - 10 + 'A');
                    
    //             kernel_print(binfo, reg_label, 0x00FFFFFF);
    //             kernel_print_hex8(binfo, c8.V[i], 0x00FFFF80);
    //         }
    //     } else {
    //         // Draw Window Outline
    //         kernel_draw_box(binfo, 60, 60, 1150, 640, 0x0050FF50);
    //         cursor_x = 550; cursor_y = 45;
    //         kernel_print(binfo, "--- CHOOSE A ROM TO RUN ---\n", 0x00FF00FF);

    //         uint32_t bar5 = pci_read_config(deviceInfo.bus, deviceInfo.device, deviceInfo.function, 0x24);
    //         uint32_t mmioBaseAddress = bar5 & ~0xF;
    //         // uint32_t pml4Index = (mmioBaseAddress >> 39) & 0x1FF;
    //         // uint32_t pdptIndex = (mmioBaseAddress >> 30) & 0x1FF;
    //         // uint32_t pageDirectoryIndex = (mmioBaseAddress >> 21) & 0x1FF;
    //         // uint32_t pageTableIndex = (mmioBaseAddress >> 12) & 0x1FF;
    //         // uint64_t pmlBaseAddress = readCr3();

    //         volatile hbaMem* _hbaMem = (hbaMem*)mmioBaseAddress;

    //         uint32_t _cap = _hbaMem->cap;
    //         uint32_t _pi = _hbaMem->pi;

    //         uint32_t bar0 = pci_read_config(deviceInfo.bus, deviceInfo.device, deviceInfo.function, 0x10);
    //         volatile uint32_t* presentStateRegAddr = (uint32_t*)(bar0 + 0x24);
    //         volatile uint32_t* capabilitiesRegAddr = (uint32_t*)(bar0 + 0x40);
    //         volatile uint32_t* controlClockRegAddr = (uint32_t*)(bar0 + 0x2C);
    //         volatile uint32_t* powerControllRegAddr = (uint32_t*)(bar0 + 0x29);
    //         volatile uint32_t* response0RegAddr = (uint32_t*)(bar0 + 0x10);
    //         volatile uint32_t* response1RegAddr = (uint32_t*)(bar0 + 0x14);
    //         volatile uint32_t* response2RegAddr = (uint32_t*)(bar0 + 0x18);
    //         volatile uint32_t* response3RegAddr = (uint32_t*)(bar0 + 0x1C);
    //         volatile uint16_t* normalInterruptStatusEnableRegAddr = (uint16_t*)(bar0 + 0x34);
    //         volatile uint16_t* commandRegAddr = (uint16_t*)(bar0 + 0x0E);
    //         volatile uint16_t* errorInteruptStatusRegAddr = (uint16_t*)(bar0 + 0x32);
    //         volatile uint8_t* hostControllRegAddr = (uint8_t*)(bar0 + 0x28);
    //         volatile uint8_t* timeoutControllRegAddr = (uint8_t*)(bar0 + 0x2E);
    //         volatile uint8_t* softwareResetRegAddr = (uint8_t*)(bar0 + 0x2F);
    //         volatile uint32_t* commandArgument1Addr = (uint32_t*)(bar0 + 0x08);
    //         volatile uint16_t* NormalInterruptStatusRegAddr = (uint16_t*)(bar0 + 0x30);
    //         volatile uint32_t* bufferDataPort = (uint32_t*)(bar0 + 0x20);
    //         volatile uint16_t* transferModeReg = (uint16_t*)(bar0 + 0x0C);

    //         cursor_x = 550; cursor_y = 90;
    //         kernel_print_int(binfo, fileCount, 0x00FF00FF);
    //         sdhciContext* context = (sdhciContext*)blockDevice.context;
    //         uint8_t* sectorData8 = (uint8_t*)context->sectorData;
    //         uint32_t* data = (uint32_t*)context->sectorData;
    //         uint8_t readBlLen = (context->csd2 >> 8) & 0xF;
    //         uint32_t cSize = ((context->csd2 & 0x3) << 10) | ((context->csd1 >> 22) & 0x3FF);
    //         uint8_t cSizeMult = (context->csd1 >> 7) & 0x7;

    //         uint64_t capacityBytes =
    //             (uint64_t)(cSize + 1) *
    //             (1ULL << (cSizeMult + 2)) *
    //             (1ULL << readBlLen);

    //         uint64_t sectorCount = capacityBytes / 512;

    //         f_readdir(&rootDir, &fileInfo);
    //         while (fileInfo.fname[0] != '\0') {
    //             cursor_x = 650; cursor_y += 20;
    //             kernel_print(binfo, fileInfo.fname, 0x00FF00FF);
    //             f_readdir(&rootDir, &fileInfo);
    //         }
    //         f_rewinddir(&rootDir);

    //         // for (uint8_t i = 0; i < fileCount - 1; i++) {
    //         //     cursor_x = 650; cursor_y += 20;
    //         //     kernel_print(binfo, (char*)fileNames[i], 0x00FF00FF);
    //         // }
    //         // cursor_x = 550; cursor_y = 130;
    //         // kernel_print_hex(binfo, *bufferDataPort, 0x00FF00FF);
    //         // cursor_x = 550; cursor_y = 150;
    //         // kernel_print_hex(binfo, *response2RegAddr, 0x00FF00FF);
    //         // cursor_x = 550; cursor_y = 170;
    //         // kernel_print_hex(binfo, *response3RegAddr, 0x00FF00FF);

    //         // for (uint8_t i = 0; i < 16; i++) {
    //         //     cursor_x = 550; cursor_y += 20;
    //         //     kernel_print_int(binfo, ((uint16_t*)565248)[i], 0x00FF00FF);
    //         // }
    //         // cursor_x = 600; cursor_y = 130;
    //         // kernel_print_int(binfo, _databaseAddress, 0x00FF00FF);
    //     }
    // }
}

void tick() {
    switch (_activeScreen) {
        case ERROR_SCREEN:
            errorScreen.tick(&errorScreen, 0);
            break;
        case CHIP_SCREEN:
            chipScreen.tick(&chipScreen, (void*)&c8);
            break;
        case FILE_SCREEN:
            fileScreen.tick(&fileScreen, &_fileData);
            break;
    }
    // if (error)
    //     return;

    // if (!runningChip8)
    //     return;
    
    // chip8_update_timers(&c8);

    // for (uint16_t i = 0; i < OPCODES_PER_FRAME; i++) {
    //     if (!c8.drewSprite)
    //         chip8_cycle(&c8);
    // }

    // if (c8.sound_timer > 0) {
    //     kernel_play_sound(440);
    // } else {
    //     kernel_stop_sound();
    // }

    // c8.drewSprite = 0;

    // framesPassed++;
}

void input() {
    switch (_activeScreen) {
        case ERROR_SCREEN:
            errorScreen.input(&errorScreen, 0);
            break;
        case CHIP_SCREEN:
            chipScreen.input(&chipScreen, (void*)&c8);
            break;
        case FILE_SCREEN:
            fileScreen.input(&fileScreen, &_fileData);
            break;
    }

    // if (error)
    //     return;

    // uint8_t live_scancode = kernel_get_scancode();
    // if (runningChip8) {
    //     switch (live_scancode) {
    //         // Row 1: 1, 2, 3, 4  ->  1, 2, 3, C
    //         case 0x02: c8.key[0x1] = 1; break; // '1' pressed
    //         case 0x82: c8.key[0x1] = 0; break; // '1' released
    //         case 0x03: c8.key[0x2] = 1; break; // '2' pressed
    //         case 0x83: c8.key[0x2] = 0; break; // '2' released
    //         case 0x04: c8.key[0x3] = 1; break; // '3' pressed
    //         case 0x84: c8.key[0x3] = 0; break; // '3' released
    //         case 0x05: c8.key[0xC] = 1; break; // '4' pressed
    //         case 0x85: c8.key[0xC] = 0; break; // '4' released

    //         // Row 2: Q, W, E, R  ->  4, 5, 6, D
    //         case 0x10: c8.key[0x4] = 1; break; // 'Q' pressed
    //         case 0x90: c8.key[0x4] = 0; break; // 'Q' released
    //         case 0x11: c8.key[0x5] = 1; break; // 'W' pressed
    //         case 0x91: c8.key[0x5] = 0; break; // 'W' released
    //         case 0x12: c8.key[0x6] = 1; break; // 'E' pressed
    //         case 0x92: c8.key[0x6] = 0; break; // 'E' released
    //         case 0x13: c8.key[0xD] = 1; break; // 'R' pressed
    //         case 0x93: c8.key[0xD] = 0; break; // 'R' released

    //         // Row 3: A, S, D, F  ->  7, 8, 9, E
    //         case 0x1E: c8.key[0x7] = 1; break; // 'A' pressed
    //         case 0x9E: c8.key[0x7] = 0; break; // 'A' released
    //         case 0x1F: c8.key[0x8] = 1; break; // 'S' pressed
    //         case 0x9F: c8.key[0x8] = 0; break; // 'S' released
    //         case 0x20: c8.key[0x9] = 1; break; // 'D' pressed
    //         case 0xA0: c8.key[0x9] = 0; break; // 'D' released
    //         case 0x21: c8.key[0xE] = 1; break; // 'F' pressed
    //         case 0xA1: c8.key[0xE] = 0; break; // 'F' released

    //         // Row 4: Z, X, C, V  ->  A, 0, B, F
    //         case 0x2C: c8.key[0xA] = 1; break; // 'Z' pressed
    //         case 0xAC: c8.key[0xA] = 0; break; // 'Z' released
    //         case 0x2D: c8.key[0x0] = 1; break; // 'X' pressed
    //         case 0xAD: c8.key[0x0] = 0; break; // 'X' released
    //         case 0x2E: c8.key[0xB] = 1; break; // 'C' pressed
    //         case 0xAE: c8.key[0xB] = 0; break; // 'C' released
    //         case 0x2F: c8.key[0xF] = 1; break; // 'V' pressed
    //         case 0xAF: c8.key[0xF] = 0; break; // 'V' released

    //         default: break;
    //     }
    // } else {
    //     if (live_scancode == 0x1C) {
    //         runningChip8 = 1;
    //     }
    // }
}

void kernel_main(BootInfo* binfo) {
    uint64_t total_pixels = binfo->Height * binfo->PixelsPerScanLine;

    initialize_physical_memory_manager(binfo, &memoryManager);
    initialize_heap(&_heap, &memoryManager, 4096);

    readFileAddr = allocate_physical_page(&memoryManager);

    _c8_start.c8 = &c8;

    initialize_screen(&errorScreen, binfo, 0, errorScreen_start, errorScreen_tick, errorScreen_input, errorScreen_render, set_screen);
    initialize_screen(&chipScreen, binfo, &_c8_start, chipScreen_start, chipScreen_tick, chipScreen_input, chipScreen_render, set_screen);
    initialize_screen(&fileScreen, binfo, 0, fileScreen_start, fileScreen_tick, fileScreen_input, fileScreen_render, set_screen);
    _activeScreen = FILE_SCREEN;
    lastActiveScreen = FILE_SCREEN;

    // _16BitAddress = (uint32_t*)heap_allocate(&_heap, 16);
    // _32BitAddress = (uint32_t*)heap_allocate(&_heap, 32);
    // _64BitAddress = (uint32_t*)heap_allocate(&_heap, 64);
    // _128BitAddress = (uint32_t*)heap_allocate(&_heap, 128);
    // _256BitAddress = (uint32_t*)heap_allocate(&_heap, 256);
    // _512BitAddress = (uint32_t*)heap_allocate(&_heap, 512);

    // heap_free(&_heap, _16BitAddress);
    // heap_free(&_heap, _64BitAddress);
    // heap_free(&_heap, _256BitAddress);

    // _16BitAddress = (uint32_t*)heap_allocate(&_heap, 513);

    pci_enumerate_all(&deviceInfo);

    int code = sdhci_initialize(&deviceInfo, &blockDevice, &memoryManager, &_heap);

    if (code != SDHCI_SUCCESS) {
        _errorHeader.errorCode = 1;
        _errorHeader._errorType = SDHCI_ERROR;
        _activeScreen = ERROR_SCREEN;
    }

    diskio_set_device(&blockDevice);

    _fileData.fatFs = &fatFs;
    _fileData.fileInfo = &fileInfo;
    _fileData.rootDir = &rootDir;

    result = f_mount(&fatFs, "0:", 1);
    result = f_opendir(&rootDir, "0:/");

    // ahciInitialized = ahci_initialize(&deviceInfo, &memoryManager, &blockDevice);

    // if (ahci_initialize) {
    //     diskio_set_device(&blockDevice);
    // }

    uint64_t cycles_per_screen_frame = 16666 * binfo->CpuMhz; // 60 FPS Frame
    uint64_t cycles_per_second = 1000000 * binfo->CpuMhz;

    uint64_t last_screen_frame_time = rdtsc();
    uint64_t last_fps_update = rdtsc();
    
    uint16_t screen_frame_counter = 0;
    uint16_t current_fps_display = 0;

    start();


    while (1) {
        uint64_t current_time = rdtsc();

        // Enforce the 60 Hz Frame Sync window
        if (current_time - last_screen_frame_time < cycles_per_screen_frame) {
            __asm__ volatile("pause");
            continue;
        }
        last_screen_frame_time = current_time;
        screen_frame_counter++;

        if (lastActiveScreen != _activeScreen) {
            switch (_activeScreen) {
                case ERROR_SCREEN:
                    errorScreen.start(&errorScreen, 0);
                    break;
                case CHIP_SCREEN:
                    // _activeScreen = ERROR_SCREEN;
                    f_rewinddir(&rootDir);
                    f_readdir(_fileData.rootDir, _fileData.fileInfo);
                    for (uint8_t i = 0; i < _fileData.selectedFile; i++) {
                        f_readdir(_fileData.rootDir, _fileData.fileInfo);
                    }

                    FIL file;

                    char path[128] = "0:/";

                    FRESULT res = f_open(&file, strcat(path, _fileData.fileInfo->fname), FA_READ);
                        
                    if (res != FR_OK) {
                        _activeScreen = ERROR_SCREEN;
                        errorScreen.start(&errorScreen, 0);

                        kernel_clear_screen(binfo, 0x0012121F);

                        kernel_print(binfo, _fileData.fileInfo->fname, 0x0000FFFF);

                        kernel_blit(binfo->Framebuffer, binfo->BackBuffer, total_pixels);

                        while (1) {}
                        break;
                    }

                    UINT bytesRead = 0;

                    res = f_read(&file, (uint8_t*)(readFileAddr), f_size(&file), &bytesRead);
                    // if (res == FR_OK) {
                    //     _activeScreen = ERROR_SCREEN;
                    //     errorScreen.start(&errorScreen, 0);
                    //     kernel_clear_screen(binfo, 0x0012121F);

                    //     cursor_x = 100; cursor_y = 100;
                    //     kernel_print_int(binfo, bytesRead, 0x0000FFFF);

                    //     cursor_x = 100; cursor_y = 120;
                    //     kernel_print_int(binfo, f_size(&file), 0x0000FFFF);

                    //     cursor_x = 100; cursor_y = 140;
                    //     kernel_print_int(binfo, ((uint8_t*)readFileAddr)[1], 0x0000FFFF);

                    //     kernel_blit(binfo->Framebuffer, binfo->BackBuffer, total_pixels);
                    //     while (1) {}
                    //     break;
                    // }

                    // errorScreen.start(&errorScreen, 0);
                    // break;

                    for (uint16_t i = 0; i < bytesRead; i++) {
                        _c8_start.rom[i] = ((uint8_t*)readFileAddr)[i];
                    }
                    _c8_start.romSize = (uint16_t)bytesRead;

                        // _activeScreen = ERROR_SCREEN;
                        // errorScreen.start(&errorScreen, 0);
                        // kernel_clear_screen(binfo, 0x0012121F);

                        // cursor_x = 100; cursor_y = 100;
                        // kernel_print_hex8(binfo, _c8_start.rom[131], 0x0000FFFF);

                        // cursor_x = 100; cursor_y = 120;
                        // kernel_print_int(binfo, f_size(&file), 0x0000FFFF);

                        // cursor_x = 100; cursor_y = 140;
                        // kernel_print_hex8(binfo, ((uint8_t*)readFileAddr)[131], 0x0000FFFF);

                        // kernel_blit(binfo->Framebuffer, binfo->BackBuffer, total_pixels);
                        // while (1) {}


                    chipScreen.start(&chipScreen, (void*)&_c8_start);
                    break;
                case FILE_SCREEN:
                    fileScreen.start(&fileScreen, 0);
                    break;
            }

            lastActiveScreen = _activeScreen;
        }
        
        input();
        tick();

        if (current_time - last_fps_update >= cycles_per_second) {
                current_fps_display = screen_frame_counter;
                screen_frame_counter = 0;
                last_fps_update = current_time;
        }

        kernel_clear_screen(binfo, 0x0012121F);

        draw(binfo);

        kernel_blit(binfo->Framebuffer, binfo->BackBuffer, total_pixels);
    }
}
