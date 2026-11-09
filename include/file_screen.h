#ifndef FILE_SCREEN_H
#define FILE_SCREEN_H

#include "screen.h"
#include "ff.h"
#include "renderer.h"

typedef struct {
    char name[128];
    UINT size;
    FILINFO* fileInfo;
} file;

typedef struct folder {
    char name[128];
    struct folder* folders;
    file* files;
    uint8_t folderCount;
    uint8_t fileCount;
    DIR* dir;
} folder;

typedef struct {
    FATFS* fatFs;
    DIR* rootDir;
    FILINFO* fileInfo;
    folder rootFolder;
    uint8_t selectedFile;
    uint8_t filesFound;
    _Bool readSelectedFile;
} fileData;


void fileScreen_start(screen* _screen, void* input) {}

void fileScreen_tick(screen* _screen, void* input) {
    fileData* _fileData = (fileData*)input;

    if (_fileData->readSelectedFile) {
        _fileData->readSelectedFile = 0;

        _screen->swap(CHIP_SCREEN);
    }
}

void fileScreen_input(screen* _screen, void* input) {
    fileData* _fileData = (fileData*)input;

    uint8_t live_scancode = kernel_get_scancode();
    if (live_scancode == 0x1C) { // enter
        _fileData->readSelectedFile = 1;
    }
    if (live_scancode == 0xCB) { // Left Arrow
        if (_fileData->selectedFile > 0)
            _fileData->selectedFile--;
    }
    if (live_scancode == 0xCD) { // Right Arrow
        if (_fileData->selectedFile < _fileData->filesFound)
            _fileData->selectedFile++;
    }
}

void fileScreen_render(screen* _screen, void* input) {
    fileData* _fileData = (fileData*)input;

    // Draw Window Outline
    kernel_draw_box(_screen->binfo, 60, 60, 1150, 640, 0x0050FF50);
    cursor_x = 550; cursor_y = 45;
    kernel_print(_screen->binfo, "--- CHOOSE A ROM TO RUN ---\n", 0x00FF00FF);

    // uint32_t bar5 = pci_read_config(deviceInfo.bus, deviceInfo.device, deviceInfo.function, 0x24);
    // uint32_t mmioBaseAddress = bar5 & ~0xF;
    // uint32_t pml4Index = (mmioBaseAddress >> 39) & 0x1FF;
    // uint32_t pdptIndex = (mmioBaseAddress >> 30) & 0x1FF;
    // uint32_t pageDirectoryIndex = (mmioBaseAddress >> 21) & 0x1FF;
    // uint32_t pageTableIndex = (mmioBaseAddress >> 12) & 0x1FF;
    // uint64_t pmlBaseAddress = readCr3();

    // volatile hbaMem* _hbaMem = (hbaMem*)mmioBaseAddress;

    // uint32_t _cap = _hbaMem->cap;
    // uint32_t _pi = _hbaMem->pi;

    // uint32_t bar0 = pci_read_config(deviceInfo.bus, deviceInfo.device, deviceInfo.function, 0x10);
    // volatile uint32_t* presentStateRegAddr = (uint32_t*)(bar0 + 0x24);
    // volatile uint32_t* capabilitiesRegAddr = (uint32_t*)(bar0 + 0x40);
    // volatile uint32_t* controlClockRegAddr = (uint32_t*)(bar0 + 0x2C);
    // volatile uint32_t* powerControllRegAddr = (uint32_t*)(bar0 + 0x29);
    // volatile uint32_t* response0RegAddr = (uint32_t*)(bar0 + 0x10);
    // volatile uint32_t* response1RegAddr = (uint32_t*)(bar0 + 0x14);
    // volatile uint32_t* response2RegAddr = (uint32_t*)(bar0 + 0x18);
    // volatile uint32_t* response3RegAddr = (uint32_t*)(bar0 + 0x1C);
    // volatile uint16_t* normalInterruptStatusEnableRegAddr = (uint16_t*)(bar0 + 0x34);
    // volatile uint16_t* commandRegAddr = (uint16_t*)(bar0 + 0x0E);
    // volatile uint16_t* errorInteruptStatusRegAddr = (uint16_t*)(bar0 + 0x32);
    // volatile uint8_t* hostControllRegAddr = (uint8_t*)(bar0 + 0x28);
    // volatile uint8_t* timeoutControllRegAddr = (uint8_t*)(bar0 + 0x2E);
    // volatile uint8_t* softwareResetRegAddr = (uint8_t*)(bar0 + 0x2F);
    // volatile uint32_t* commandArgument1Addr = (uint32_t*)(bar0 + 0x08);
    // volatile uint16_t* NormalInterruptStatusRegAddr = (uint16_t*)(bar0 + 0x30);
    // volatile uint32_t* bufferDataPort = (uint32_t*)(bar0 + 0x20);
    // volatile uint16_t* transferModeReg = (uint16_t*)(bar0 + 0x0C);

    cursor_x = 550; cursor_y = 90;
    // kernel_print_int(_screen->binfo, fileCount, 0x00FF00FF);
    // sdhciContext* context = (sdhciContext*)blockDevice.context;
    // uint8_t* sectorData8 = (uint8_t*)context->sectorData;
    // uint32_t* data = (uint32_t*)context->sectorData;
    // uint8_t readBlLen = (context->csd2 >> 8) & 0xF;
    // uint32_t cSize = ((context->csd2 & 0x3) << 10) | ((context->csd1 >> 22) & 0x3FF);
    // uint8_t cSizeMult = (context->csd1 >> 7) & 0x7;

    // uint64_t capacityBytes =
    //     (uint64_t)(cSize + 1) *
    //     (1ULL << (cSizeMult + 2)) *
    //     (1ULL << readBlLen);

    // uint64_t sectorCount = capacityBytes / 512;

    // f_readdir(_fileData->rootDir, _fileData->fileInfo);
    // while (_fileData->fileInfo->fname[0] != '\0') {
    //     cursor_x = 650; cursor_y += 20;
    //     kernel_print(_screen->binfo, _fileData->fileInfo->fname, 0x00FF00FF);
    //     f_readdir(_fileData->rootDir, _fileData->fileInfo);
    // }
    // f_rewinddir(_fileData->rootDir);

    cursor_x = 100; cursor_y = 75;
    uint16_t x_multiplier = 0;
    uint16_t y_multiplier = 0;
    _fileData->filesFound = 0;
    f_readdir(_fileData->rootDir, _fileData->fileInfo);
    while (_fileData->fileInfo->fname[0] != '\0') {
        if (x_multiplier % 9 == 0 && x_multiplier != 0) {
            x_multiplier = 0;
            y_multiplier++;
        }

        if ((_fileData->fileInfo->fattrib & AM_DIR) == 0) {
            // Draw File
            kernel_draw_box(_screen->binfo, 100 + (x_multiplier * 125), 75 + (y_multiplier * 125), 75, 100, 0x00FF00FF); 
            renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 85 + (y_multiplier * 125), 50, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 95 + (y_multiplier * 125), 25, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 140 + (x_multiplier * 125), 95 + (y_multiplier * 125), 25, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 105 + (y_multiplier * 125), 55, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 115 + (y_multiplier * 125), 35, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 150 + (x_multiplier * 125), 115 + (y_multiplier * 125), 15, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 125 + (y_multiplier * 125), 55, 0x00FF00FF);
            renderer_draw_line(_screen->binfo, 150 + (x_multiplier * 125), 135 + (y_multiplier * 125), 15, 0x00FF00FF);
        } else {
            // Draw Folder
            kernel_draw_box(_screen->binfo, 100 + (x_multiplier * 125), 95 + (y_multiplier * 125), 75, 75, 0x00FF00FF); 
            kernel_draw_box(_screen->binfo, 100 + (x_multiplier * 125), 75 + (y_multiplier * 125), 50, 20, 0x00FF00FF); 
        }

        cursor_x = 100 + (x_multiplier * 125); cursor_y = 180 + (y_multiplier * 125);
        kernel_print(_screen->binfo, _fileData->fileInfo->fname, 0x00FF00FF);

        x_multiplier++;
        f_readdir(_fileData->rootDir, _fileData->fileInfo);
        cursor_x = 100; cursor_y = 75;
        _fileData->filesFound++;
    } 
    f_rewinddir(_fileData->rootDir);
    _fileData->filesFound--;

    x_multiplier = _fileData->selectedFile;

    if (x_multiplier >= 9) {
        while (x_multiplier >= 9) {
            x_multiplier -= 9;
            y_multiplier++;
        }
        
    }

    kernel_draw_box(_screen->binfo, 95 + (x_multiplier * 125), 70 + (y_multiplier * 125), 85, 110, 0x0000FF); 

    // for (uint8_t i = 0; i < fileCount - 1; i++) {
    //     cursor_x = 650; cursor_y += 20;
    //     kernel_print(binfo, (char*)fileNames[i], 0x00FF00FF);
    // }
    // cursor_x = 550; cursor_y = 130;
    // kernel_print_hex(binfo, *bufferDataPort, 0x00FF00FF);
    // cursor_x = 550; cursor_y = 150;
    // kernel_print_hex(binfo, *response2RegAddr, 0x00FF00FF);
    // cursor_x = 550; cursor_y = 170;
    // kernel_print_hex(binfo, *response3RegAddr, 0x00FF00FF);

    // for (uint8_t i = 0; i < 16; i++) {
    //     cursor_x = 550; cursor_y += 20;
    //     kernel_print_int(binfo, ((uint16_t*)565248)[i], 0x00FF00FF);
    // }
    // cursor_x = 600; cursor_y = 130;
    // kernel_print_int(binfo, _databaseAddress, 0x00FF00FF);
}

#endif