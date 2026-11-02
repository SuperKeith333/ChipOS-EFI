#ifndef BOOTINFO_H
#define BOOTINFO_H

#include "memory_map.h"

typedef struct {
    unsigned int* Framebuffer;
    unsigned int* BackBuffer;
    unsigned long long Width;
    unsigned long long Height;
    unsigned long long PixelsPerScanLine;
    unsigned long long CpuMhz;
    unsigned char* KernelWorkspace;
    MemoryMapEntry* memoryMap;
    unsigned long long memoryMapSize;
    unsigned long long descriptorSize;
} BootInfo;

#endif
