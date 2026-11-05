#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include "bootinfo.h"
#include "io.h"


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    EFI_STATUS status;

    // Get Graphics Output Protocol (GOP) for drawing
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    
    if (EFI_ERROR(status)) {
        Print(L"Failed to locate GOP\n");
        return status;
    }

    // Calculate the required memory for a single display buffer
    uint64_t bufferSize = gop->Mode->Info->HorizontalResolution * gop->Mode->Info->VerticalResolution * sizeof(unsigned int);
    uint64_t pagesNeeded = (bufferSize + 4095) / 4096;

    EFI_PHYSICAL_ADDRESS backBufferAddress;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, pagesNeeded, &backBufferAddress);

    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate back buffer memory pages\n");
        return status;
    }

    // Calculate the number of cpu cyles that occur in a second
    unsigned long long tsc_start = rdtsc();
    uefi_call_wrapper(BS->Stall, 1, 100000);
    unsigned long long tsc_end = rdtsc();

    // Get the required memory for the memory map

    UINTN originalMemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
    UINTN mapKey = 0;
    UINTN descriptorSize = 0;
    UINT32 descriptorVersion = 0;

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &originalMemoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);

    if (status != EFI_BUFFER_TOO_SMALL) {
        Print(L"EFI buffer does not have its expected value!\n");
        return status;
    }

    // Calculate the number of required memory pages

    UINTN memoryMapBufferSizeInBytes = originalMemoryMapSize + (2 * descriptorSize);
    UINTN memoryMapBufferSizeInPages = (memoryMapBufferSizeInBytes + 4095) / 4096;

    EFI_PHYSICAL_ADDRESS memoryMapAddress;

    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, memoryMapBufferSizeInPages, &memoryMapAddress);

    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate memory for the memory map!\n");
        return status;
    }

    memoryMap = (EFI_MEMORY_DESCRIPTOR*)memoryMapAddress;

    // Get the actually memory map address w/ the adjusted space

    UINTN memoryMapSize = memoryMapBufferSizeInBytes;
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &memoryMapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);

    if (EFI_ERROR(status)) {
        Print(L"Failed to get memory map!\n");
        return status;
    }

    // 2. Prepare boot info to pass to the kernel
    BootInfo info;
    info.Framebuffer = (unsigned int*)gop->Mode->FrameBufferBase;
    info.BackBuffer = (unsigned int*)backBufferAddress;
    info.backBufferSize = pagesNeeded * 4096;
    info.Width = gop->Mode->Info->HorizontalResolution;
    info.Height = gop->Mode->Info->VerticalResolution;
    info.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
    info.CpuMhz = (tsc_end - tsc_start) / 100000;
    info.memoryMap = (MemoryMapEntry*)memoryMap;
    info.memoryMapSize = memoryMapSize;
    info.descriptorSize = descriptorSize;

    if (info.CpuMhz == 0) info.CpuMhz = 2000;

   

    // 3. Exit Boot Services 

    status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, mapKey);

    if (EFI_ERROR(status)) {
        Print(L"Failed to exit boot services!\n");
        return status;
    }
    
    // Call the kernel
    extern void kernel_main(BootInfo* binfo);
    kernel_main(&info);

    return EFI_SUCCESS;
}
