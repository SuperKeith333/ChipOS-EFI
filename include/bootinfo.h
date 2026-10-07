#ifndef BOOTINFO_H
#define BOOTINFO_H

typedef struct {
    unsigned int* Framebuffer;
    unsigned int* BackBuffer;
    unsigned long long Width;
    unsigned long long Height;
    unsigned long long PixelsPerScanLine;
    unsigned long long CpuMhz;
    unsigned char* KernelWorkspace;
} BootInfo;

#endif
