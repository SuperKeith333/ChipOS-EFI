#include "drivers/storage/ahci.h"

#include "pci.h"
#include "io.h"

_Bool ahci_initialize(pciDeviceInfo* deviceInfo) {
    uint32_t bar5 = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x24);
    uint32_t mmioBaseAddress = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x24) & ~0xF;
    uint32_t pml4Index = (mmioBaseAddress >> 39) & 0x1FF;
    uint32_t pdptIndex = (mmioBaseAddress >> 30) & 0x1FF;
    uint32_t pageDirectoryIndex = (mmioBaseAddress >> 21) & 0x1FF;
    uint32_t pageTableIndex = (mmioBaseAddress >> 12) & 0x1FF;

    uint64_t pmlBaseAddress = readCr3();
}