#ifndef SDHCI_H
#define SDHCI_H

#include "pci.h"
#include "drivers/storage/block_device.h"

_Bool sdhci_initialize(pciDeviceInfo* deviceInfo, BlockDevice* _blockDevice);

#endif