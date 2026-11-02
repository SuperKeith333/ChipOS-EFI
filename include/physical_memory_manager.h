#ifndef PHYSICAL_MEMORY_MANAGER_H
#define PHYSICAL_MEMORY_MANAGER_H

#include <stdint.h>
#include "memory_map.h"
#include "bootinfo.h"

#define MAX_MEMORY_REGIONS 32

typedef struct {
    MemoryMapEntry original;
    unsigned long long nextFreeAddress;
    unsigned long long remainingPages;
    unsigned long long regionStart;
    unsigned long long regionPages;
    unsigned long long bitmapOffset;
    unsigned long long bitmapBytes;
} physicalMemoryRegion;

typedef struct {
    physicalMemoryRegion memoryRegions[MAX_MEMORY_REGIONS];
    uint8_t usableRegions;
    uint64_t freePages;
    uint8_t bitmapStorage[4096];
    uint64_t bitmapUsedBytes;
} physicalMemoryManager;



void initialize_physical_memory_manager(BootInfo* binfo, physicalMemoryManager* manager);
void free_physical_page(physicalMemoryManager* manager, unsigned long long physicalAddress);
unsigned long long allocate_physical_page(physicalMemoryManager* manager);

#endif