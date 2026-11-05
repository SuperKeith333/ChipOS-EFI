#include "drivers/storage/ahci.h"

#include "pci.h"

_Bool ahci_initialize(pciDeviceInfo* deviceInfo) {
    uint32_t bar5 = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x24) & ~0xF;
}