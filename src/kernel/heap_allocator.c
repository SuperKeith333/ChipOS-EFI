#include "heap_allocator.h"

#include <stdint.h>
#include <stddef.h>

#include "physical_memory_manager.h"

_Bool initialize_heap(heap* heap, physicalMemoryManager* memoryManager, uint32_t size) {
    if (size == 0)
        return 0;
    if (size > UINT32_MAX - 4095)
        return 0;

    uint64_t pageCount = (size + 4095) / 4096;

    uint64_t allocationResult = allocate_continuous_pages(memoryManager, pageCount);

    if (allocationResult == 0)
        return 0;
    
    heap->startAddress = allocationResult;
    heap->memoryManager = memoryManager;
    heap->totalSize = pageCount * 4096;

    heap->firstBlock = (heapBlock*)allocationResult;
    heap->firstBlock->size = heap->totalSize - sizeof(heapBlock);
    heap->firstBlock->isFree = 1;
    heap->firstBlock->nextBlock = NULL;
    heap->firstBlock->prevBlock = NULL;

    return 1;
}