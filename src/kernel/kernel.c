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
#include "screen.h"

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

}

void kernel_main(BootInfo* binfo) {
    uint64_t total_pixels = binfo->Height * binfo->PixelsPerScanLine;

    initialize_physical_memory_manager(binfo, &memoryManager);
    _Bool res = initialize_heap(&_heap, &memoryManager, 65536);

    readFileAddr = allocate_physical_page(&memoryManager);

    _c8_start.c8 = &c8;

    initialize_screen(&errorScreen, binfo, 0, errorScreen_start, errorScreen_tick, errorScreen_input, errorScreen_render, set_screen);
    initialize_screen(&chipScreen, binfo, &_c8_start, chipScreen_start, chipScreen_tick, chipScreen_input, chipScreen_render, set_screen);
    initialize_screen(&fileScreen, binfo, 0, fileScreen_start, fileScreen_tick, fileScreen_input, fileScreen_render, set_screen);
    _activeScreen = FILE_SCREEN;
    lastActiveScreen = FILE_SCREEN;

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

    _fileData.rootFolder.name[0] = '0';
    _fileData.rootFolder.name[1] = ':';
    _fileData.rootFolder.name[2] = '\0';

    _fileData.rootFolder.path[0] = '0';
    _fileData.rootFolder.path[1] = ':';
    _fileData.rootFolder.path[2] = '/';
    _fileData.rootFolder.path[3] = '\0';
    _fileData.rootFolder.dir = &rootDir;
    _fileData.currentFolder = &_fileData.rootFolder;
    _fileData.rootFolder.generated = 1;
    
    f_readdir(_fileData.rootDir, _fileData.fileInfo);
    while (_fileData.fileInfo->fname[0] != '\0') {
        if ((_fileData.fileInfo->fattrib & AM_DIR) == 0) {
            _fileData.rootFolder.fileCount++;
        } else {
            _fileData.rootFolder.folderCount++;
        }
        f_readdir(_fileData.rootDir, _fileData.fileInfo);
    }

    file files[_fileData.rootFolder.fileCount];
    folder folders[_fileData.rootFolder.folderCount];

    f_rewinddir(_fileData.rootDir); 
    uint8_t fileI = 0;
    uint8_t folderI = 0;
    uint8_t iterator = 0;
    f_readdir(_fileData.rootDir, _fileData.fileInfo);
    while (_fileData.fileInfo->fname[0] != '\0') {
        if ((_fileData.fileInfo->fattrib & AM_DIR) == 0) {
            FILINFO* fileInfoPointer = heap_allocate(&_heap, sizeof(FILINFO));
            memcpy(fileInfoPointer, _fileData.fileInfo, sizeof(FILINFO));
            files[fileI].fileInfo = fileInfoPointer;
            for (uint16_t i = 0; i < 256; i++) {
                files[fileI].name[i] = _fileData.fileInfo->fname[i];
            }
            files[fileI].index = iterator;
            fileI++;
        } else {
            for (uint16_t i = 0; i < 256; i++) {
                folders[folderI].name[i] = _fileData.fileInfo->fname[i];
            }
            folders[folderI].parentFolder = &_fileData.rootFolder;
            folders[folderI].generated = 0;
            folders[folderI].index = iterator;
            folderI++;
        }
        f_readdir(_fileData.rootDir, _fileData.fileInfo);
        iterator++;
    }

    folder* folderPtr = heap_allocate(&_heap, sizeof(folder) * (_fileData.rootFolder.folderCount));
    memcpy(folderPtr, folders, sizeof(folder) * _fileData.rootFolder.folderCount);
    _fileData.rootFolder.folders = folderPtr; 

    file* filesPtr = heap_allocate(&_heap, sizeof(file) * _fileData.rootFolder.fileCount);
    memcpy(filesPtr, files, sizeof(file) * _fileData.rootFolder.fileCount);
    _fileData.rootFolder.files = filesPtr; 

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
                    if (_fileData.selectedFile <= _fileData.currentFolder->fileCount - 1) {
                        FIL file;

                        char path[256];
                        memcpy(path, _fileData.currentFolder->path, sizeof(_fileData.currentFolder->path) + 1);

                        FRESULT res = f_open(&file, strcat(path, _fileData.currentFolder->files[_fileData.selectedFile].name), FA_READ);

                        UINT bytesRead = 0;

                        res = f_read(&file, (uint8_t*)(readFileAddr), f_size(&file), &bytesRead);

                        for (uint16_t i = 0; i < bytesRead; i++) {
                            _c8_start.rom[i] = ((uint8_t*)readFileAddr)[i];
                        }
                        _c8_start.romSize = (uint16_t)bytesRead;

                        chipScreen.start(&chipScreen, (void*)&_c8_start);
                    } else {
                        _Bool generated = 0;
                        for (uint8_t i = 0; i < _fileData.currentFolder->folderCount; i++) {
                            if (strcmp(_fileData.currentFolder->folders[i].name, _fileData.currentFolder->folders[_fileData.selectedFile - _fileData.currentFolder->fileCount].name) == 0) {
                                if (_fileData.currentFolder->folders[i].generated) {
                                    _activeScreen = FILE_SCREEN;
                                    generated = 1;
                                    _fileData.currentFolder = &_fileData.currentFolder->folders[i];
                                    _fileData.selectedFile = 0;
                                    break;
                                }
                                DIR dir;
                                char dirName[256];
                                memcpy(dirName, _fileData.currentFolder->name, strlen(_fileData.currentFolder->name) + 1);
                                strcat(dirName, "/");
                                strcat(dirName, _fileData.currentFolder->folders[_fileData.selectedFile - _fileData.currentFolder->fileCount].name);
                                strcat(dirName, "/");

                                FRESULT res = f_opendir(&dir, dirName);
                                DIR* dirPtr = heap_allocate(&_heap, sizeof(DIR));
                                memcpy(dirPtr, &dir, sizeof(DIR));

                                _fileData.currentFolder->folders[i].dir = dirPtr;
                                memcpy(_fileData.currentFolder->folders[i].path, dirName, strlen(dirName) + 1);
                                _fileData.currentFolder = &_fileData.currentFolder->folders[i];
                            }
                        }

                        if (generated)
                            break;

                        f_rewinddir(_fileData.currentFolder->dir);
                        _fileData.currentFolder->fileCount = 0;
                        _fileData.currentFolder->folderCount = 0;
                        f_readdir(_fileData.currentFolder->dir, _fileData.fileInfo);
                        while (_fileData.fileInfo->fname[0] != '\0') {
                            if ((_fileData.fileInfo->fattrib & AM_DIR) == 0) {
                                _fileData.currentFolder->fileCount++;
                            } else {
                                _fileData.currentFolder->folderCount++;
                            }
                            f_readdir(_fileData.currentFolder->dir, _fileData.fileInfo);
                        }


                        file newFiles[_fileData.currentFolder->fileCount];
                        folder newFolders[_fileData.currentFolder->folderCount];



                        f_rewinddir(_fileData.currentFolder->dir); 
                        fileI = 0;
                        folderI = 0;
                        f_readdir(_fileData.currentFolder->dir, _fileData.fileInfo);
                        while (_fileData.fileInfo->fname[0] != '\0') {
                            if ((_fileData.fileInfo->fattrib & AM_DIR) == 0) {
                                FILINFO* fileInfoPointer = heap_allocate(&_heap, sizeof(FILINFO));
                                memcpy(fileInfoPointer, _fileData.fileInfo, sizeof(FILINFO));
                                newFiles[fileI].fileInfo = fileInfoPointer;
                                for (uint16_t i = 0; i < 256; i++) {
                                    newFiles[fileI].name[i] = _fileData.fileInfo->fname[i];
                                }
                                fileI++;
                            } else {
                                for (uint16_t i = 0; i < 256; i++) {
                                    newFolders[folderI].name[i] = _fileData.fileInfo->fname[i];
                                }
                                newFolders[folderI].parentFolder = _fileData.currentFolder;
                                folderI++;
                            }
                            f_readdir(_fileData.currentFolder->dir, _fileData.fileInfo);
                        }

                        folder* folderPtr = heap_allocate(&_heap, sizeof(folder) * (_fileData.currentFolder->folderCount));
                        memcpy(folderPtr, newFolders, sizeof(folder) * _fileData.currentFolder->folderCount);
                        _fileData.currentFolder->folders = folderPtr; 

                        file* filesPtr = heap_allocate(&_heap, sizeof(file) * _fileData.currentFolder->fileCount);
                        memcpy(filesPtr, newFiles, sizeof(file) *_fileData.currentFolder->fileCount);
                        _fileData.currentFolder->files = filesPtr; 

                        _fileData.selectedFile = 0;
                        _fileData.currentFolder->generated = 1;

                        _activeScreen = FILE_SCREEN;
                    }
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
