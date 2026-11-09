#ifndef FILE_SCREEN_H
#define FILE_SCREEN_H

#include "screen.h"
#include "ff.h"
#include "renderer.h"
#include <stdint.h>
#include <stddef.h>

typedef struct {
    TCHAR name[256];
    FILINFO* fileInfo;
    uint8_t index;
} file;

typedef struct folder {
    char name[256];
    char path[256];
    struct folder* folders;
    struct folder* parentFolder;
    uint8_t index;
    _Bool generated;
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
    folder* currentFolder;
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
    if (live_scancode == 0x81) { // escape
        if (_fileData->currentFolder->parentFolder != NULL)
            _fileData->currentFolder = _fileData->currentFolder->parentFolder;
    }
    if (live_scancode == 0x1C) { // enter
        _fileData->readSelectedFile = 1;
    }
    if (live_scancode == 0xCB) { // Left Arrow
        if (_fileData->selectedFile > 0)
            _fileData->selectedFile--;
    }
    if (live_scancode == 0xCD) { // Right Arrow
        if (_fileData->selectedFile < _fileData->currentFolder->fileCount + _fileData->currentFolder->folderCount - 1)
            _fileData->selectedFile++;
    }
}

void fileScreen_render(screen* _screen, void* input) {
    fileData* _fileData = (fileData*)input;

    // Draw Window Outline
    kernel_draw_box(_screen->binfo, 60, 60, 1150, 640, 0x0050FF50);
    cursor_x = 550; cursor_y = 45;
    kernel_print(_screen->binfo, "--- CHOOSE A ROM TO RUN ---\n", 0x00FF00FF);

    cursor_x = 100; cursor_y = 75;
    uint16_t x_multiplier = 0;
    uint16_t y_multiplier = 0;
    _fileData->filesFound = 0;
    for (uint8_t i = 0; i < _fileData->currentFolder->fileCount; i++) {
        if (x_multiplier % 9 == 0 && x_multiplier != 0) {
            x_multiplier = 0;
            y_multiplier++;
        }

        kernel_draw_box(_screen->binfo, 100 + (x_multiplier * 125), 75 + (y_multiplier * 125), 75, 100, 0x00FF00FF); 
        renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 85 + (y_multiplier * 125), 50, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 95 + (y_multiplier * 125), 25, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 140 + (x_multiplier * 125), 95 + (y_multiplier * 125), 25, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 105 + (y_multiplier * 125), 55, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 115 + (y_multiplier * 125), 35, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 150 + (x_multiplier * 125), 115 + (y_multiplier * 125), 15, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 110 + (x_multiplier * 125), 125 + (y_multiplier * 125), 55, 0x00FF00FF);
        renderer_draw_line(_screen->binfo, 150 + (x_multiplier * 125), 135 + (y_multiplier * 125), 15, 0x00FF00FF);

        cursor_x = 100 + (x_multiplier * 125); cursor_y = 180 + (y_multiplier * 125);
        kernel_print(_screen->binfo, _fileData->currentFolder->files[i].name, 0x00FF00FF);
        x_multiplier++;
    } 
    for (uint8_t i = 0; i < _fileData->currentFolder->folderCount; i++) {
        if (x_multiplier % 9 == 0 && x_multiplier != 0) {
            x_multiplier = 0;
            y_multiplier++;
        }

        kernel_draw_box(_screen->binfo, 100 + (x_multiplier * 125), 95 + (y_multiplier * 125), 75, 75, 0x00FF00FF); 
        kernel_draw_box(_screen->binfo, 100 + (x_multiplier * 125), 75 + (y_multiplier * 125), 50, 20, 0x00FF00FF); 

        cursor_x = 100 + (x_multiplier * 125); cursor_y = 180 + (y_multiplier * 125);
        kernel_print(_screen->binfo, _fileData->currentFolder->folders[i].name, 0x00FF00FF);
        x_multiplier++;
    } 

    x_multiplier = _fileData->selectedFile;

    if (x_multiplier >= 9) {
        while (x_multiplier >= 9) {
            x_multiplier -= 9;
            y_multiplier++;
        }
        
    }

    kernel_draw_box(_screen->binfo, 95 + (x_multiplier * 125), 70 + (y_multiplier * 125), 85, 110, 0x0000FF); 
}

#endif