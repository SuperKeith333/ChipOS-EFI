#ifndef ERROR_SCREEN_H
#define ERROR_SCREEN_H

#include "renderer.h"
#include "screen.h"

typedef enum {
    NONE,
    BLOCK_DEVICE_ERROR,
    SDHCI_ERROR
} errorType;


typedef struct {
    errorType _errorType;
    int errorCode;
} errorHeader;

void errorScreen_start(screen* _screen, void* input) {}
void errorScreen_input(screen* _screen, void* input) {}
void errorScreen_tick(screen* _screen, void* input) {}

void errorScreen_render(screen* _screen, void* input) {
    errorHeader* header = (errorHeader*)input;

    kernel_draw_box(_screen->binfo, 200, 60, 875, 200, 0x00FFFFFF);

    switch (header->_errorType)
    {
        case BLOCK_DEVICE_ERROR:
            cursor_x = 600; cursor_y = 70;
            kernel_print(_screen->binfo, "BLOCK_DEVICE_ERROR", 0x0000FFFF);
            switch (header->errorCode)
            {
                case 1:
                    cursor_x = 580; cursor_y = 100;
                    kernel_print(_screen->binfo, "BLOCK_DEVICE_DEVICE_ERROR", 0x0000FFFF);
                    break;
                case 2:
                    cursor_x = 560; cursor_y = 100;
                    kernel_print(_screen->binfo, "BLOCK_DEVICE_DEVICE_NOT_READY", 0x0000FFFF);
                    break;
                case 3:
                    cursor_x = 580; cursor_y = 100;
                    kernel_print(_screen->binfo, "BLOCK_DEVICE_INVALID_LBA", 0x0000FFFF);
                    break;
                case 4:
                    cursor_x = 560; cursor_y = 100;
                    kernel_print(_screen->binfo, "BLOCK_DEVICE_INVALID_ARGUMENT", 0x0000FFFF);
                    break;
                case 5:
                    cursor_x = 595; cursor_y = 100;
                    kernel_print(_screen->binfo, "BLOCK_DEVICE_TIMEOUT", 0x0000FFFF);
                    break;
                case 6:
                    cursor_x = 590; cursor_y = 100;
                    kernel_print(_screen->binfo, "BLOCK_DEVICE_IO_ERROR", 0x0000FFFF);
                    break;
                    
                default:
                    break;
            }
            break;
        case SDHCI_ERROR:
            cursor_x = 620; cursor_y = 70;
            kernel_print(_screen->binfo, "SDHCI_ERROR", 0x0000FFFF);
            switch (header->errorCode)
            {
                case 1:
                    cursor_x = 560; cursor_y = 100;
                    kernel_print(_screen->binfo, "SDHCI_SOFTWARE_RESET_TIMEOUT", 0x0000FFFF);
                    break;
                case 2:
                    cursor_x = 520; cursor_y = 100;
                    kernel_print(_screen->binfo, "SDHCI_INTERNAL_CLOCK_STABILITY_TIMEOUT", 0x0000FFFF);
                    break;
                case 3:
                    cursor_x = 520; cursor_y = 100;
                    kernel_print(_screen->binfo, "SDHCI_FAILED_TO_ALLOCATE_HEAP_MEMORY", 0x0000FFFF);
                    break;
                case 4:
                    cursor_x = 580; cursor_y = 100;
                    kernel_print(_screen->binfo, "SDHCI_GENERIC_TIMEOUT", 0x0000FFFF);
                    break;
                    
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

#endif