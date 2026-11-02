#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

typedef struct {
    unsigned int type;
    unsigned int pad;
    unsigned long long physicalStart;
    unsigned long long virtualStart;
    unsigned long long numberOfPages;
    unsigned long long attribute;
} MemoryMapEntry;

#endif