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

uint64_t allocate_continuous_pages(physicalMemoryManager* manager, uint32_t pages) {
    if (pages == 0)
        return 0;

    unsigned long long pagePhysicalStart = 0;
    for (unsigned long long i = 0; i < manager->usableRegions; i++) {
        if (manager->memoryRegions[i].remainingPages >= pages) {
            uint32_t freePages = 0;
            uint32_t startingPage = 0;

            for (uint32_t pageIndex = 0; pageIndex < manager->memoryRegions[i].regionPages; pageIndex++) {
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;
                if (!(manager->bitmapStorage[globalBitmapIndex] & (1 << bitIndex))) {
                    freePages++;
                    if (freePages >= pages) {
                        startingPage = pageIndex - pages + 1;
                        break;
                    }
                } else {
                    freePages = 0;
                }
            }

            if (freePages != pages)
                continue;

            for (unsigned long long pageIndex = startingPage; pageIndex < startingPage + pages; pageIndex++) {
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;
                    
                manager->bitmapStorage[globalBitmapIndex] = (manager->bitmapStorage[globalBitmapIndex] | (1 << bitIndex));  
                manager->memoryRegions[i].remainingPages--;    
                manager->freePages--;

                if (pageIndex == startingPage)
                    pagePhysicalStart = manager->memoryRegions[i].regionStart + (pageIndex * 4096);
                
                
            }

            return pagePhysicalStart;
        }
    }

    return pagePhysicalStart;
}

_Bool free_continuous_pages(physicalMemoryManager* manager, uint64_t physicalAddress, uint32_t pages) {
    if (pages == 0)
        return 0;
    if ((physicalAddress % 4096) != 0)
        return 0;

    for (unsigned long long i = 0; i < manager->usableRegions; i++) {
        if (physicalAddress >= manager->memoryRegions[i].regionStart && physicalAddress < manager->memoryRegions[i].regionStart + (manager->memoryRegions[i].regionPages * 4096)) {
            _Bool allPagesAllocated = 1;
            uint64_t offset = physicalAddress - manager->memoryRegions[i].regionStart;

            if (offset / 4096 + pages > manager->memoryRegions[i].regionPages)
                continue;

            uint32_t endingPage = offset / 4096 + pages;

            for (uint32_t pageIndex = offset / 4096; pageIndex < endingPage; pageIndex++) {

                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;
                if (!(manager->bitmapStorage[globalBitmapIndex] & (1 << bitIndex))) {
                    allPagesAllocated = 0;
                }
            }
                
            if (allPagesAllocated) {
                for (uint32_t pageIndex = offset / 4096; pageIndex < endingPage; pageIndex++) {
                    unsigned long long byteIndex = pageIndex / 8;
                    unsigned long long bitIndex = pageIndex % 8;
                    unsigned long long globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;
                    manager->bitmapStorage[globalBitmapIndex] = (manager->bitmapStorage[globalBitmapIndex] & ~(1 << bitIndex));
                    manager->memoryRegions[i].remainingPages++;
                    manager->freePages++;
                }

                return 1;
            }

        }
    }

    return 0;
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

_Bool reserve_physical_memory(physicalMemoryManager* manager, uint64_t physicalAddress, uint64_t sizeInBytes) {
    if (sizeInBytes == 0)
        return 0; // Failure
    if (physicalAddress > UINT64_MAX - sizeInBytes)
        return 0; // Failure

    uint64_t offset = (physicalAddress % 4096);

    if (sizeInBytes > UINT64_MAX - offset - 4095)
        return 0;

    uint64_t totalNumOfPages = (offset + sizeInBytes + 4095) / 4096;
    uint64_t physicalEnd = physicalAddress + sizeInBytes;

    for (uint64_t i = 0; i < manager->usableRegions; i++) {
        if (physicalAddress >= manager->memoryRegions[i].regionStart && physicalEnd <= manager->memoryRegions[i].regionStart + (manager->memoryRegions[i].regionPages * 4096)) {
            uint64_t firstPageIndex = (physicalAddress - manager->memoryRegions[i].regionStart) / 4096;
            
            for (uint64_t pageIndex = firstPageIndex; pageIndex < firstPageIndex + totalNumOfPages; pageIndex++) {
                uint64_t byteIndex = pageIndex / 8;
                uint64_t bitIndex = pageIndex % 8;
                uint64_t globalBitmapIndex = manager->memoryRegions[i].bitmapOffset + byteIndex;

                if (!(manager->bitmapStorage[globalBitmapIndex] & (1 << bitIndex))) {
                    manager->bitmapStorage[globalBitmapIndex] = (manager->bitmapStorage[globalBitmapIndex] | (1 << bitIndex));
                    manager->memoryRegions[i].remainingPages--;
                    manager->freePages--;
                }
            }

            return 1; // Success
        }
    }

    return 0; // Failure
}