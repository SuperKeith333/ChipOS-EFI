#include "drivers/storage/sdhci.h"

#include <stddef.h>

#include "physical_memory_manager.h"
#include "drivers/storage/block_device.h"
#include "helpers.h"

sdhciAddresses _sdhciAddresses;

sdhciStatus sdhci_initialize(pciDeviceInfo* deviceInfo, BlockDevice* _blockDevice, physicalMemoryManager* memoryManager, heap* _heap) {
    uint32_t bar0 = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x10);
    _sdhciAddresses.softwareResetAddress = (uint8_t*)(bar0 + 0x2F);
    _sdhciAddresses.presentStateRegAddr = (uint32_t*)(bar0 + 0x24);
    _sdhciAddresses.capabilitiesRegAddr = (uint32_t*)(bar0 + 0x40);
    _sdhciAddresses.controlClockRegAddr = (uint32_t*)(bar0 + 0x2C);
    _sdhciAddresses.NormalInterruptStatusRegAddr = (uint16_t*)(bar0 + 0x30);
    _sdhciAddresses.normalInterruptSignalEnableRegAddr = (uint16_t*)(bar0 + 0x38);
    _sdhciAddresses.response0RegAddr = (uint32_t*)(bar0 + 0x10);
    _sdhciAddresses.response1RegAddr = (uint32_t*)(bar0 + 0x14);
    _sdhciAddresses.response2RegAddr = (uint32_t*)(bar0 + 0x18);
    _sdhciAddresses.response3RegAddr = (uint32_t*)(bar0 + 0x1C);
    _sdhciAddresses.blockSizeRegAddr = (uint16_t*)(bar0 + 0x04);
    _sdhciAddresses.blockCountRegAddr = (uint16_t*)(bar0 + 0x06);
    _sdhciAddresses.transferModeReg = (uint16_t*)(bar0 + 0x0C);
    _sdhciAddresses.bufferDataPort = (uint32_t*)(bar0 + 0x20);
    _sdhciAddresses.errorInteruptStatusRegAddr = (uint16_t*)(bar0 + 0x32);
    _sdhciAddresses.powerControllRegAddr = (uint8_t*)(bar0 + 0x29);
    _sdhciAddresses.hostControllRegAddr = (uint8_t*)(bar0 + 0x28);
    _sdhciAddresses.timeoutControllRegAddr = (uint8_t*)(bar0 + 0x2E);
    _sdhciAddresses.normalInterruptStatusEnableRegAddr = (uint16_t*)(bar0 + 0x34);
    _sdhciAddresses.errorInterruptStatusEnableRegAddr = (uint16_t*)(bar0 + 0x36);
    _sdhciAddresses.commandRegAddr = (uint16_t*)(bar0 + 0x0E);
    _sdhciAddresses.commandArgument1Addr = (uint32_t*)(bar0 + 0x08);

    uint32_t count = 0;

    setByteAtIndex_uint8(_sdhciAddresses.softwareResetAddress, 0, 1); // Starts softare reset
    if (!checkForTimeout_uint8(_sdhciAddresses.softwareResetAddress, 0, 0, 1, 0)) // Check if reset was successful
        return SDHCI_SOFTWARE_RESET_TIMEOUT;


    setBitAtOffset_uint32(_sdhciAddresses.controlClockRegAddr, 0, 1); // Enable the SD Card Internal Clock
    if (!checkForTimeout_uint32(_sdhciAddresses.controlClockRegAddr, 0, -1, 1, 1)) // Check if the clock started
        return SDHCI_INTERNAL_CLOCK_STABILITY_TIMEOUT;

    setBitAtOffset_uint32(_sdhciAddresses.controlClockRegAddr, 8, 128); // sets the SD clock divisor to 128
    setBitAtOffset_uint32(_sdhciAddresses.controlClockRegAddr, 1, 2); // enables the clock signal

    setBitAtOffset_uint8(_sdhciAddresses.powerControllRegAddr, 0, 0x0E); // sets the sd card power supply voltage to 3.3V
    setBitAtOffset_uint8(_sdhciAddresses.powerControllRegAddr, 0, 1); // enables sd card power

    *_sdhciAddresses.timeoutControllRegAddr = 0x0E; // set the sd timeout control to 14
    *_sdhciAddresses.normalInterruptStatusEnableRegAddr = 0xFFFF; // enable all normal interrupt status bits
    *_sdhciAddresses.errorInterruptStatusEnableRegAddr = 0xFFFF; // enable all error interript status bits
    *_sdhciAddresses.commandArgument1Addr = 0x00000000; // clears the command argument
    *_sdhciAddresses.commandRegAddr = 0x0000; // clears the register

    void* contextAddr = heap_allocate(_heap, sizeof(sdhciContext));
    if (contextAddr == NULL)
        return SDHCI_FAILED_TO_ALLOCATE_HEAP_MEMORY;
    sdhciContext* context = (sdhciContext*)contextAddr;

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;

    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001; // reset the normal interrupt status
    *_sdhciAddresses.commandArgument1Addr = 0x000001AA; // Checks if the card supports 2.7-3.6V
    *_sdhciAddresses.commandRegAddr = 0x081A; // Gets the SD Physical Layer specification

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;
    
    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001; // reset the normal interrupt status
    *_sdhciAddresses.commandArgument1Addr = 0x00000000; // clears the command argument
    *_sdhciAddresses.commandRegAddr = 0x371A; // tell the SD card the next command is an application-specific command (ACMD)

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;
    
    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001; // reset the normal interrupt status
    *_sdhciAddresses.commandArgument1Addr = 0x40FF8000; // operating condition bitmaps (requests high-capacity support and requests the supported voltage ranges)
    *_sdhciAddresses.commandRegAddr = 0x2902; // sends ACMD41

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;

    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001; // reset the normal interrupt status
    *_sdhciAddresses.commandArgument1Addr = 0x00000000; // clears the command argument
    *_sdhciAddresses.commandRegAddr = 0x0201; // asks the sd card for the card identification register (CID)

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;

    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001; // reset the normal interrupt status
    *_sdhciAddresses.commandArgument1Addr = 0x00000000; // clears the command argument
    *_sdhciAddresses.commandRegAddr = 0x031A; // asks the sd card for a relative card address (RCA)

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;

    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001; // reset the normal interrupt status
    uint16_t RCA = (uint16_t)(*_sdhciAddresses.response0RegAddr >> 16);
    *_sdhciAddresses.commandArgument1Addr = (RCA << 16); // places the RCA in the upper 16 bits (discarding the upper 16 & using the lower 16)
    *_sdhciAddresses.commandRegAddr = 0x0901; // requests the card's cards specific data register

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;

    
    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0003; // completly clears the normal interrupt stats (0s it out)
    *_sdhciAddresses.errorInteruptStatusRegAddr = 0x0003;
    context->csd0 = *_sdhciAddresses.response0RegAddr;
    context->csd1 = *_sdhciAddresses.response1RegAddr;
    context->csd2 = *_sdhciAddresses.response2RegAddr;
    context->csd3 = *_sdhciAddresses.response3RegAddr;
    *_sdhciAddresses.commandArgument1Addr = (uint32_t)(RCA << 16); // Places the rest of the RCA in the upper 16 bits
    *_sdhciAddresses.commandRegAddr = 0x071B; // transition the card from standby to transfer mode
    
    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return SDHCI_GENERIC_TIMEOUT;
    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return SDHCI_GENERIC_TIMEOUT;

    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0xFFFF; // zero out the register (without knowing the previous result)
    *_sdhciAddresses.errorInteruptStatusRegAddr = 0xFFFF; // zero out the register (without knowing the previous result)

    uint64_t sectorData = allocate_physical_page(memoryManager);

    uint8_t readBlLen = (context->csd2 >> 8) & 0xF;
    uint32_t cSize = ((context->csd2 & 0x3) << 10) | ((context->csd1 >> 22) & 0x3FF);
    uint8_t cSizeMult = (context->csd1 >> 7) & 0x7;
    uint64_t capacityBytes = (uint64_t)(cSize + 1) * (1ULL << (cSizeMult + 2)) * (1ULL << readBlLen);
    uint64_t sectorCount = capacityBytes / 512;
    context->sectorCount = sectorCount;

    context->sectorData = sectorData;
    context->bar0 = bar0;

    sdhci_read_sectors(context, 0, 1, (void*)sectorData);

    _blockDevice->context = context;
    return SDHCI_SUCCESS;
}

BlockDeviceStatus sdhci_read_sectors(void* context, uint32_t lba, uint32_t count, void* buffer) {
    sdhciContext* _sdhciContext = (sdhciContext*)context;

    if (count != 1)
        return BLOCK_DEVICE_INVALID_ARGUMENT;
    if (lba >= _sdhciContext->sectorCount)
        return BLOCK_DEVICE_INVALID_ARGUMENT;
    if (count > _sdhciContext->sectorCount)
        return BLOCK_DEVICE_INVALID_ARGUMENT;
    
    *_sdhciAddresses.NormalInterruptStatusRegAddr = 0x0001;
    
    *_sdhciAddresses.commandArgument1Addr = lba * 512;
    *_sdhciAddresses.blockSizeRegAddr = 512;
    *_sdhciAddresses.blockCountRegAddr = 1;
    *_sdhciAddresses.transferModeReg = 0x0012;

    setBitAtOffset_uint16(_sdhciAddresses.normalInterruptStatusEnableRegAddr, 5, 1); // allows the control to report to buffer ready ready bit
    setBitAtOffset_uint16(_sdhciAddresses.normalInterruptSignalEnableRegAddr, 5, 1); // allows cpu interrupt

    if (!checkForTimeout_uint32(_sdhciAddresses.presentStateRegAddr, 0, 0, 1, 0)) // wait for the controller to finish operations
        return BLOCK_DEVICE_TIMEOUT;

    *_sdhciAddresses.commandRegAddr = 0x113A; // read a 512 byte block from lba

    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return BLOCK_DEVICE_TIMEOUT;

    for (uint8_t i = 0; i < 128; i++) {
        ((uint32_t*)buffer)[i] = *_sdhciAddresses.bufferDataPort;
    }

    if (!checkForTimeout_uint16(_sdhciAddresses.NormalInterruptStatusRegAddr, 0, 0, 1, 1)) // wait for the controller to complete the command
        return BLOCK_DEVICE_TIMEOUT;

    return BLOCK_DEVICE_SUCCESS;
}