#ifndef AHCI_H
#define AHCI_H

#include <stddef.h>

#include "pci.h"
#include "physical_memory_manager.h"
#include "drivers/storage/block_device.h"


extern uint64_t* _virtualIdentityDeviceBuffer;
extern uint32_t _databaseAddress;

typedef struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t _reserved;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t devslp;
    uint8_t _[56];
} hbaPort;

typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint8_t _reserved1[20];
    uint32_t bohc;
    uint8_t _reserved2[212];
    hbaPort portReigsters[32];
} hbaMem;

typedef struct {
    uint16_t flags;
    uint16_t prdtLength;
    uint32_t prdbc;
    uint8_t _reserved[16];
    uint32_t ctba;
    uint32_t ctbau;
} ahciCommandHeader;

typedef struct {
    uint32_t databaseAddress;
    uint32_t databaseAddressUpper;
    uint8_t _reserved1[4];
    uint32_t byteCountAndInterupt;
} ahciPrdtEntry;

typedef struct {
    uint8_t commandFis[64];
    uint8_t _reserved1[64];
    ahciPrdtEntry prdtEntries[1];
} ahciCommandTable;

_Static_assert(sizeof(hbaPort) == 0x80, "Incorrect hbaPort size");
_Static_assert(sizeof(ahciCommandHeader) == 0x20, "Incorrect AHCI command header size");
_Static_assert(sizeof(hbaMem) == 0x1100, "Incorrect hbaMem size");
_Static_assert(sizeof(ahciPrdtEntry) == 16, "Incorrect AHCI prdt entry size");
_Static_assert(sizeof(ahciCommandTable) == 144, "Incorrect AHCI command table size");
_Static_assert(offsetof(ahciCommandTable, prdtEntries) == 0x80, "Incorrect PRDT offset");
_Static_assert(offsetof(hbaPort, ci) == 0x38, "Incorrect CI offset");
_Static_assert(offsetof(hbaPort, sntf) == 0x3C, "Incorrect SNTF offset");
_Static_assert(offsetof(hbaMem, portReigsters) == 0x100, "Incorrect port offset");

_Bool ahci_initialize(pciDeviceInfo* deviceInfo, physicalMemoryManager* memoryManager, BlockDevice* blockDevice);

#endif