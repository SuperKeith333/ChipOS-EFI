#ifndef HEAP_ALLOCATOR_H
#define HEAP_ALLOCATOR_H

#include <stdint.h>

#include "physical_memory_manager.h"

typedef struct heapBlock {
    uint64_t size;
    _Bool isFree;
    struct heapBlock* nextBlock;
    struct heapBlock* prevBlock;
} heapBlock;


typedef struct {
    uint64_t startAddress;
    uint64_t totalSize;
    heapBlock* firstBlock;
    physicalMemoryManager* memoryManager;
} heap;

_Bool initialize_heap(heap* heap, physicalMemoryManager* memoryManager, uint32_t size);
void* heap_allocate(heap* _heap, uint32_t size);
_Bool heap_free(heap* _heap, void* address);

#endif