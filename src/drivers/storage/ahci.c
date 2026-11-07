#include "drivers/storage/ahci.h"

#include <stdint.h>

#include "pci.h"
#include "io.h"
#include "physical_memory_manager.h"
#include "drivers/storage/diskio_adapter.h"

uint64_t* _virtualIdentityDeviceBuffer;
uint32_t _databaseAddress;

_Bool ahci_initialize(pciDeviceInfo* deviceInfo, physicalMemoryManager* memoryManager, BlockDevice* blockDevice) {
    uint32_t bar5 = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x24);
    uint32_t mmioBaseAddress = bar5 & ~0xF;
    // uint32_t pml4Index = (mmioBaseAddress >> 39) & 0x1FF;
    // uint32_t pdptIndex = (mmioBaseAddress >> 30) & 0x1FF;
    // uint32_t pageDirectoryIndex = (mmioBaseAddress >> 21) & 0x1FF;
    // uint32_t pageTableIndex = (mmioBaseAddress >> 12) & 0x1FF;
    // uint64_t pmlBaseAddress = readCr3();

    volatile hbaMem* _hbaMem = (hbaMem*)mmioBaseAddress;

    uint32_t _cap = _hbaMem->cap;
    uint32_t _pi = _hbaMem->pi;

    if ((_pi & 1) == 0)
        return 0;

    if ((_hbaMem->portReigsters[0].cmd >> 15) & 1)
        return 0;
    if ((_hbaMem->portReigsters[0].cmd >> 14) & 1)
        return 0;
    if (_hbaMem->portReigsters[0].cmd & 1)
        return 0;
    if ((_hbaMem->portReigsters[0].cmd >> 4) & 1)
        return 0;

    uint64_t commandListAddress = allocate_continuous_pages(memoryManager, 2);

    if (commandListAddress == 0)
        return 0;

    uint64_t* virtualCommandListAddress = (uint64_t*)commandListAddress;
    uint64_t fisRecieveAddress = commandListAddress + 4096;
    uint64_t* virtualFisRecieveAddress = (uint64_t*)fisRecieveAddress;

    for (uint16_t i = 0; i < 1024; i++) {
        virtualCommandListAddress[i] = 0;
    }

    _hbaMem->portReigsters[0].clb = (uint32_t)commandListAddress;
    _hbaMem->portReigsters[0].clbu = (uint32_t)(commandListAddress >> 32);
    _hbaMem->portReigsters[0].fb = (uint32_t)fisRecieveAddress;
    _hbaMem->portReigsters[0].fbu = (uint32_t)(fisRecieveAddress >> 32);

    ahciCommandHeader* _ahciCommandHeader = (ahciCommandHeader*)commandListAddress;
    // for (uint8_t i = 0; i < 32; i++) {
        
    // }

    uint64_t commandTableAddress = allocate_physical_page(memoryManager);
    if (commandTableAddress == 0)
        return 0;

    ahciCommandTable* _ahciCommandTable = (ahciCommandTable*)commandTableAddress;
    uint64_t* virtualAhciCommandTableAddress = (uint64_t*)commandTableAddress;
    for (uint16_t i = 0; i < 512; i++) {
        virtualAhciCommandTableAddress[i] = 0;
    }

    
    _ahciCommandHeader[0].ctba = (uint32_t)commandTableAddress;
    _ahciCommandHeader[0].ctbau = (uint32_t)(commandTableAddress >> 32);

    _ahciCommandHeader[0].flags = 5;
    _ahciCommandHeader[0].prdtLength = 1;

    uint64_t identityDeviceBuffer = allocate_physical_page(memoryManager);
    if (identityDeviceBuffer == 0)
        return 0;
    uint64_t* virtualIdentityDeviceBuffer = (uint64_t*)identityDeviceBuffer;
    _virtualIdentityDeviceBuffer = virtualIdentityDeviceBuffer;


    _ahciCommandTable->prdtEntries[0].databaseAddress = (uint32_t)identityDeviceBuffer;
    _ahciCommandTable->prdtEntries[0].databaseAddressUpper = (uint32_t)(identityDeviceBuffer >> 32);
    _ahciCommandTable->prdtEntries[0].byteCountAndInterupt = 511;
    _ahciCommandTable->commandFis[0] = 0x27;
    _ahciCommandTable->commandFis[1] = 0x80;
    _ahciCommandTable->commandFis[2] = 0xEC;

    _databaseAddress = _hbaMem->portReigsters[0].serr;

    for (uint8_t i = 3; i < 20; i++) {
        _ahciCommandTable->commandFis[i] = 0;
    }

    for (uint8_t i = 0; i < 64; i++) {
        virtualIdentityDeviceBuffer[i] = 0;
    }


    _hbaMem->portReigsters[0].cmd |= (1 << 4);

    uint32_t counter = 0;
    while (((_hbaMem->portReigsters[0].cmd >> 14) & 1) == 0) {
        counter++;

        if (counter >= 1000000)
            return 0;
    }

    _hbaMem->portReigsters[0].cmd |= 1;

    counter = 0;
    while (((_hbaMem->portReigsters[0].cmd >> 15) & 1) == 0) {
        counter++;

        if (counter >= 1000000)
            return 0;
    }

    counter = 0;
    while (((_hbaMem->portReigsters[0].tfd >> 7) & 1) != 0) {
        counter++;

        if (counter >= 1000000) {
            _databaseAddress = 1;
            return 0;
        }
    }

    counter = 0;
    while (((_hbaMem->portReigsters[0].tfd >> 3) & 1) != 0) {
        counter++;

        if (counter >= 1000000) {
            _databaseAddress = 2;
            return 0;
        }
    }



    _hbaMem->portReigsters[0].serr = 0xFFFFFFFF;
    _hbaMem->portReigsters[0].ci = 1;


    counter = 0;
    while ((_hbaMem->portReigsters[0].ci & 1) != 0) {
        counter++;

        if (counter >= 1000000) {
            _databaseAddress = _hbaMem->portReigsters[0].is;
            return 0;
        }
    }

    if (((_hbaMem->portReigsters[0].is >> 30) & 1) != 0)
        return 0;
    

    return 1;
}