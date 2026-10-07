#include <efi.h>
#include <efilib.h>
#include "bootinfo.h"

static inline unsigned long long rdtsc(void) {
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)hi << 32) | lo;
}


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    EFI_STATUS status;

    // 1. Get Graphics Output Protocol (GOP) for drawing
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    
    if (EFI_ERROR(status)) {
        Print(L"Failed to locate GOP\n");
        return status;
    }

    UINTN bufferSize = gop->Mode->Info->HorizontalResolution * gop->Mode->Info->VerticalResolution * sizeof(unsigned int);
    UINTN pagesNeeded = (bufferSize + 4095) / 4096;

    EFI_PHYSICAL_ADDRESS backBufferAddress;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, pagesNeeded, &backBufferAddress);

    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate back buffer memory pages\n");
        return status;
    }

    UINTN pagesNeededForKenelData = 16;
    EFI_PHYSICAL_ADDRESS kernelDataAddress;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, pagesNeededForKenelData, &kernelDataAddress);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate safe Kernel workspace in memory\n");
        return status;
    }

    unsigned long long tsc_start = rdtsc();
    uefi_call_wrapper(BS->Stall, 1, 100000);
    unsigned long long tsc_end = rdtsc();

    // 2. Prepare boot info to pass to the kernel
    BootInfo info;
    info.Framebuffer = (unsigned int*)gop->Mode->FrameBufferBase;
    info.BackBuffer = (unsigned int*)backBufferAddress;
    info.Width = gop->Mode->Info->HorizontalResolution;
    info.Height = gop->Mode->Info->VerticalResolution;
    info.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
    info.CpuMhz = (tsc_end - tsc_start) / 100000;
    info.KernelWorkspace = (unsigned char*)kernelDataAddress;

    if (info.CpuMhz == 0) info.CpuMhz = 2000;

    Print(L"GOP Initialized. Exiting boot services and jumping to kernel...\n");

    // 3. Exit Boot Services (Crucial: Handing full control to our OS)
    // In a production loader, you must fetch the memory map first.
    UINTN MapKey = 0;
    // For this minimal example, we bypass checking the map key to jump straight to the kernel
    uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);

    // 4. Cast an arbitrary memory address where your kernel resides, or call it directly
    // (For absolute simplicity, assume the kernel code is compiled directly with the loader or loaded manually)
    void (*kernel_entry)(BootInfo*) = (void (*)(BootInfo*))0x100000; // Hypothetical kernel address
    
    // For this single-file demonstration, we will just call our kernel function directly:
    extern void kernel_main(BootInfo* binfo);
    kernel_main(&info);

    return EFI_SUCCESS;
}
