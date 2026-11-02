#include "physical_memory_manager.h"
#include "bootinfo.h"

void initialize_physical_memory_manager(BootInfo* binfo, physicalMemoryManager* manager) {
    manager->usableRegions = 0;
    manager->freePages = 0;
    manager->bitmapUsedBytes = 0;

    for (unsigned long long i = 0; i < 4096; i++) {
        manager->bitmapStorage[i] = 0;
    }

    unsigned long long numberOfDescriptors = binfo->memoryMapSize / binfo->descriptorSize;
    for (unsigned long long  i = 0; i < numberOfDescriptors; i++) {
        unsigned char* memoryMapInBytes = ((unsigned char*)binfo->memoryMap) + (i * binfo->descriptorSize);
        MemoryMapEntry* adjustedMemoryMap = (MemoryMapEntry*)memoryMapInBytes;

        if (adjustedMemoryMap->type == 7 && adjustedMemoryMap->physicalStart != 0) {
            if (manager->usableRegions < MAX_MEMORY_REGIONS && manager->bitmapUsedBytes + (adjustedMemoryMap->numberOfPages + 7) / 8 <= 4096) {
                manager->memoryRegions[manager->usableRegions].original = *adjustedMemoryMap;
                manager->memoryRegions[manager->usableRegions].nextFreeAddress = adjustedMemoryMap->physicalStart;
                manager->memoryRegions[manager->usableRegions].remainingPages = adjustedMemoryMap->numberOfPages;
                manager->memoryRegions[manager->usableRegions].regionStart = adjustedMemoryMap->physicalStart;
                manager->memoryRegions[manager->usableRegions].regionPages = adjustedMemoryMap->numberOfPages;
                manager->memoryRegions[manager->usableRegions].bitmapBytes = (adjustedMemoryMap->numberOfPages + 7) / 8;
                manager->memoryRegions[manager->usableRegions].bitmapOffset = manager->bitmapUsedBytes;
                manager->bitmapUsedBytes += manager->memoryRegions[manager->usableRegions].bitmapBytes;
                manager->usableRegions++;
                manager->freePages += adjustedMemoryMap->numberOfPages;
            }
        }
    }
}

uint64_t allocate_physical_page(physicalMemoryManager* manager) {
    unsigned long long pagePhysicalStart = 0;
    for (unsigned long long i = 0; i < manager->usableRegions; i++) {
        if (manager->memoryRegions[i].remainingPages > 0) {
            for (unsigned long long pageIndex = 0; pageIndex < manager->memoryRegions[i].regionPages; pageIndex++) {
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;
                if (!(manager->bitmapStorage[globalBitmapIndex] & (1 << bitIndex))) {
                    manager->bitmapStorage[globalBitmapIndex] = (manager->bitmapStorage[globalBitmapIndex] | (1 << bitIndex));
                    manager->memoryRegions[i].remainingPages--;
                    manager->freePages--;

                    pagePhysicalStart = manager->memoryRegions[i].regionStart + (pageIndex * 4096);
                    return pagePhysicalStart;
                }
            }
        }
    }

    return pagePhysicalStart;
}

void free_physical_page(physicalMemoryManager* manager, uint64_t physicalAddress) {
    if (physicalAddress % 4096 == 0) {
        for (unsigned long long i = 0; i < manager->usableRegions; i++) {
            unsigned long long physicalEnd = manager->memoryRegions[i].regionStart + (manager->memoryRegions[i].regionPages * 4096);
            if (physicalAddress >= manager->memoryRegions[i].regionStart && physicalAddress < physicalEnd) {
                unsigned long long pageIndex = (physicalAddress - manager->memoryRegions[i].regionStart) / 4096;
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;
                
                if (manager->bitmapStorage[globalBitmapIndex] & (1 << bitIndex)) {
                    manager->bitmapStorage[globalBitmapIndex] = (manager->bitmapStorage[globalBitmapIndex] & ~(1 << bitIndex));

                    manager->memoryRegions[i].remainingPages++;
                    manager->freePages++;
                    return;
                }
            }
        
        }
    }
}

void reserve_physical_memory(physicalMemoryManager* manager, uint64_t physicalAddress, uint64_t sizeInBytes) {
    uint64_t page = (sizeInBytes + 4095) / 4096;
}