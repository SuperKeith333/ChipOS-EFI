#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    uint32_t* Framebuffer; // Also front buffer
    uint32_t* BackBuffer;
    uint64_t backBufferSize;
    uint64_t Width; // width of screen
    uint64_t Height; // height of the screen
    uint64_t PixelsPerScanLine;
    uint64_t CpuMhz;
    MemoryMapEntry* memoryMap;
    uint64_t memoryMapSize;
    uint64_t descriptorSize;
} BootInfo;

#endif
