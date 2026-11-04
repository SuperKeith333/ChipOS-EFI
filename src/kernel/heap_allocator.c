#include "heap_allocator.h"

#include <stdint.h>
#include <stddef.h>

#include "physical_memory_manager.h"

_Bool initialize_heap(heap* _heap, physicalMemoryManager* memoryManager, uint32_t size) {
    if (size == 0)
        return 0;
    if (size > UINT32_MAX - 4095)
        return 0;

    uint64_t pageCount = (size + 4095) / 4096;

    uint64_t allocationResult = allocate_continuous_pages(memoryManager, pageCount);

    if (allocationResult == 0)
        return 0;
    
    _heap->startAddress = allocationResult;
    _heap->memoryManager = memoryManager;
    _heap->totalSize = pageCount * 4096;

    _heap->firstBlock = (heapBlock*)allocationResult;
    _heap->firstBlock->size = _heap->totalSize - sizeof(heapBlock);
    _heap->firstBlock->isFree = 1;
    _heap->firstBlock->nextBlock = NULL;
    _heap->firstBlock->prevBlock = NULL;

    return 1;
}

void* heap_allocate(heap* _heap, uint32_t size) {
    if (size == 0)
        return NULL;

    heapBlock* currentBlockPointer = (heapBlock*)_heap->firstBlock;

    while (currentBlockPointer != NULL) {
        if (currentBlockPointer->isFree && currentBlockPointer->size >= size) {
            currentBlockPointer->isFree = 0;
            uint64_t originalBlockSize = currentBlockPointer->size;
            currentBlockPointer->size = size;

            if (originalBlockSize - size >= (sizeof(heapBlock) + 1)) {
                heapBlock* newBlock = (heapBlock*)((uint8_t*)currentBlockPointer + sizeof(heapBlock) + size);

                newBlock->size = originalBlockSize - (sizeof(heapBlock) + size);
                newBlock->isFree = 1;
                newBlock->prevBlock = currentBlockPointer;
                newBlock->nextBlock = currentBlockPointer->nextBlock;

                if (newBlock->nextBlock != NULL) {
                    newBlock->nextBlock->prevBlock = newBlock;
                }

                currentBlockPointer->nextBlock = newBlock;
            } else {
                currentBlockPointer->size = originalBlockSize;
            }

            return (void*)(currentBlockPointer + 1);
        } else {
            currentBlockPointer = currentBlockPointer->nextBlock;
        }
    }

    return NULL;
}

_Bool heap_free(heap* _heap, void* address) {
    if (_heap == NULL)
        return 0;
    if (address == 0)
        return 0;

    heapBlock* currentBlockPointer = (heapBlock*)_heap->firstBlock;

    while (currentBlockPointer != NULL) {
        if ((void*)(currentBlockPointer+1) == address) {
            if (currentBlockPointer->isFree)
                return 0;
            currentBlockPointer->isFree = 1;
            heapBlock* nextBlock = currentBlockPointer->nextBlock;

            if (nextBlock != NULL) {
                if (nextBlock->isFree) {
                    currentBlockPointer->size += nextBlock->size + sizeof(heapBlock);
                    currentBlockPointer->nextBlock = nextBlock->nextBlock;

                    if (currentBlockPointer->nextBlock != NULL) {
                        currentBlockPointer->nextBlock->prevBlock = currentBlockPointer;
                    }
                }

            }

            heapBlock* prevBlock = currentBlockPointer->prevBlock;

            if (prevBlock != NULL) {
                if (prevBlock->isFree) {
                    prevBlock->size += currentBlockPointer->size + sizeof(heapBlock);
                    prevBlock->nextBlock = currentBlockPointer->nextBlock;

                    if (prevBlock->nextBlock != NULL) {
                        prevBlock->nextBlock->prevBlock = prevBlock;
                    }
                }
            }

            return 1;
        } else {
            currentBlockPointer = currentBlockPointer->nextBlock;
        }
    }

    return 0;
}