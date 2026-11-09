#ifndef SDHCI_H
#define SDHCI_H

#include "pci.h"
#include "drivers/storage/block_device.h"
#include "physical_memory_manager.h"
#include "heap_allocator.h"

typedef struct {
    uint64_t sectorData;
    uint64_t sectorCount;
    uint32_t bar0;
    uint32_t csd0;
    uint32_t csd1;
    uint32_t csd2;
    uint32_t csd3;
} sdhciContext;

typedef struct {
    volatile uint8_t* softwareResetAddress;
    volatile uint32_t* presentStateRegAddr;
    volatile uint32_t* capabilitiesRegAddr;
    volatile uint32_t* controlClockRegAddr;
    volatile uint16_t* NormalInterruptStatusRegAddr;
    volatile uint16_t* normalInterruptSignalEnableRegAddr;
    volatile uint32_t* response0RegAddr;
    volatile uint32_t* response1RegAddr;
    volatile uint32_t* response2RegAddr;
    volatile uint32_t* response3RegAddr;
    volatile uint16_t* blockSizeRegAddr;
    volatile uint16_t* blockCountRegAddr;
    volatile uint16_t* transferModeReg;
    volatile uint32_t* bufferDataPort;
    volatile uint16_t* errorInteruptStatusRegAddr;
    volatile uint8_t* powerControllRegAddr;
    volatile uint8_t* hostControllRegAddr;
    volatile uint8_t* timeoutControllRegAddr;
    volatile uint16_t* normalInterruptStatusEnableRegAddr;
    volatile uint16_t* errorInterruptStatusEnableRegAddr;
    volatile uint16_t* commandRegAddr;
    volatile uint32_t* commandArgument1Addr;
} sdhciAddresses;

typedef enum {
    SDHCI_SUCCESS,
    SDHCI_SOFTWARE_RESET_TIMEOUT,
    SDHCI_INTERNAL_CLOCK_STABILITY_TIMEOUT,
    SDHCI_FAILED_TO_ALLOCATE_HEAP_MEMORY,
    SDHCI_GENERIC_TIMEOUT
} sdhciStatus;

extern sdhciAddresses _sdhciAddresses;

sdhciStatus sdhci_initialize(pciDeviceInfo* deviceInfo, BlockDevice* _blockDevice, physicalMemoryManager* memoryManager, heap* _heap);
BlockDeviceStatus sdhci_read_sectors(void* context, uint64_t lba, uint32_t count, void* buffer);

#endif