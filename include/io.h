#ifndef IO_H
#define IO_H

#include <stdint.h>

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint32_t inl(unsigned short port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(unsigned short port, uint32_t val) {
    __asm__ volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi)); // Run the rdtsc instruction and store the lower 32 bits in lo and the upper 32 bits in hi
    return ((uint64_t)hi << 32) | lo; // returns to the total number of cpu cycles since the computer turned on
}

static inline uint64_t readCr3(void) {
    uint64_t result = 0;

    __asm__ volatile ("mov %%cr3, %0" : "=r"(result));
    return result;
}


static inline void debug_char(char c) {
    outb(0xE9, c);
}

static void debug_hex(uint32_t v) {
    for (int i = 28; i >= 0; i -= 4)
        outb(0xE9, "0123456789ABCDEF"[(v >> i) & 0xF]);
    outb(0xE9, '\n');
}
#endif