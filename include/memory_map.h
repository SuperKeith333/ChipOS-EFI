#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

typedef struct {
    unsigned int type;
    unsigned int pad;
    unsigned long long physicalStart; // Address on physical stick of ram
    unsigned long long virtualStart; // Address according to OS
    unsigned long long numberOfPages;
    unsigned long long attribute;
} MemoryMapEntry;

#endif