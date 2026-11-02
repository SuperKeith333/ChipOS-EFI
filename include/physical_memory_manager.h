#ifndef PHYSICAL_MEMORY_MANAGER_H
#define PHYSICAL_MEMORY_MANAGER_H

#include <stdint.h>
#include "memory_map.h"
#include "bootinfo.h"

#define MAX_MEMORY_REGIONS 32

typedef struct {
    MemoryMapEntry original;
    uint64_t nextFreeAddress;
    uint64_t remainingPages;
    uint64_t regionStart;
    uint64_t regionPages;
    uint64_t bitmapOffset;
    uint64_t bitmapBytes;
} physicalMemoryRegion;

typedef struct {
    physicalMemoryRegion memoryRegions[MAX_MEMORY_REGIONS];
    uint8_t usableRegions;
    uint64_t freePages;
    uint8_t bitmapStorage[4096]; // Used to store which memory pages are allocated or free
    uint64_t bitmapUsedBytes;
} physicalMemoryManager;



void initialize_physical_memory_manager(BootInfo* binfo, physicalMemoryManager* manager);
void free_physical_page(physicalMemoryManager* manager, uint64_t physicalAddress);
uint64_t allocate_physical_page(physicalMemoryManager* manager);
_Bool reserve_physical_memory(physicalMemoryManager* manager, uint64_t physicalAddress, uint64_t sizeInBytes);

#endif