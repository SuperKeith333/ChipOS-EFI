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

#endif