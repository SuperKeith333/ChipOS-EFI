#include "drivers/storage/sdhci.h"

_Bool sdhci_initialize(pciDeviceInfo* deviceInfo, BlockDevice* _blockDevice) {
    uint32_t bar0 = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x10);
    volatile uint8_t* softwareResetAddress = (uint8_t*)(bar0 + 0x2F);
    volatile uint32_t* presentStateRegAddr = (uint32_t*)(bar0 + 0x24);
    volatile uint32_t* capabilitiesRegAddr = (uint32_t*)(bar0 + 0x40);
    volatile uint32_t* controlClockRegAddr = (uint32_t*)(bar0 + 0x2C);

    softwareResetAddress[0] = 1;

    uint32_t count = 0;
    while ((softwareResetAddress[0] & 1) != 0) {
        count++;

        if (count >= 100000)
            return 0;
    }

    *controlClockRegAddr |= (1 << 0);   

    count = 0;
    while (((controlClockRegAddr[0] >> 1) & 1) != 1) {
        count++;

        if (count >= 100000)
            return 0;
    }

    *(uint16_t*)(controlClockRegAddr) |= (128 << 8);
    *(uint16_t*)(controlClockRegAddr) |= (1 << 2);

    count = 0;
    while (((controlClockRegAddr[0] >> 2) & 1) == 1) {
        count++;

        if (count >= 100000)
            break;
    }

    volatile uint8_t* powerControllRegAddr = (uint8_t*)(bar0 + 0x29);
    volatile uint8_t* powerControllRegAddr = (uint8_t*)(bar0 + 0x29);
    *powerControllRegAddr |= (0x0E << 0);
    *powerControllRegAddr |= (1 << 0); 

    return 1;
}