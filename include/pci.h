#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#include "io.h"
#include "renderer.h"
#include "bootinfo.h"

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    _Bool foundDevice;
} pciDeviceInfo;

static inline uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t regOffset) {
    uint32_t configAddress = 0;
    configAddress = ((uint32_t)1 << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | ((uint32_t)function << 8) | ((uint32_t)regOffset & 0xFC);

    outl(0xCF8, configAddress);
    return inl(0xCFC);
}

static inline void pci_enumerate_bus(uint8_t bus, pciDeviceInfo* deviceInfo) {
    for (uint8_t device = 0; device < 32; device++) {
        uint32_t config = pci_read_config(bus, device, 0, 0x00);
        uint16_t vendorID = config & 0xFFFF;


        if (vendorID != 0xFFFF) {
            uint16_t deviceID = (config >> 16) & 0xFFFF;
            uint32_t classConfig = pci_read_config(bus, device, 0, 0x08);
            uint8_t baseClass = (classConfig >> 24) & 0xFF;
            uint8_t subClass = (classConfig >> 16) & 0xFF;
            uint8_t programInterface = (classConfig >> 8) & 0xFF;

            uint32_t headerConfig = pci_read_config(bus, device, 0, 0x0C);
            uint8_t headerType = (headerConfig >> 16) & 0xFF;
            
            if (headerType & 0x80) {
                uint8_t lastY = cursor_y;
                for (uint8_t i = 1; i < 8; i++) {
                    uint32_t funcConfig = pci_read_config(bus, device, i, 0x00);
                    uint16_t funcVendorID = funcConfig & 0xFFFF;
                    if (funcVendorID == 0xFFFF)
                        continue;

                    uint16_t funcDeviceID = (funcConfig >> 16) & 0xFFFF;
                    uint32_t funcClassConfig = pci_read_config(bus, device, i, 0x08);
                    uint8_t funcBaseClass = (funcClassConfig >> 24) & 0xFF;
                    uint8_t funcSubClass = (funcClassConfig >> 16) & 0xFF;
                    uint8_t funcProgramInterface = (funcClassConfig >> 8) & 0xFF;

                    uint32_t funcHeaderConfig = pci_read_config(bus, device, i, 0x0C);
                    uint8_t funcHeaderType = (funcHeaderConfig >> 16) & 0xFF;

                    if (funcBaseClass == 1 && funcProgramInterface == 1 && funcSubClass == 6) {
                        deviceInfo->bus = bus;
                        deviceInfo->device = device;
                        deviceInfo->function = i;
                        deviceInfo->foundDevice = 1;
                        return;
                    }

                    uint8_t lastX = cursor_x;

                    // cursor_x = 550;
                    // kernel_print_int(binfo, bus, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, device, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, i, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, funcVendorID, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, funcDeviceID, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, funcBaseClass, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, funcSubClass, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, funcProgramInterface, 0x00FF00FF);
                    // cursor_x += 20;
                    // kernel_print_int(binfo, funcHeaderType & 0x80, 0x00FF00FF);
                    // cursor_x = lastX;
                    // cursor_y += 20;
                }

                cursor_y = lastY;
            }

            if (baseClass == 1 && programInterface == 1 && subClass == 6) {
                deviceInfo->bus = bus;
                deviceInfo->device = device;
                deviceInfo->function = 0;
                deviceInfo->foundDevice = 1;
                return;
            }
            
            // cursor_x = 20;
            // kernel_print_int(binfo, deviceID, 0x00FF00FF);
            // cursor_x += 20;
            // kernel_print_int(binfo, vendorID, 0x00FF00FF);
            // cursor_x += 20;
            // kernel_print_int(binfo, baseClass, 0x00FF00FF);
            // cursor_x += 20;
            // kernel_print_int(binfo, subClass, 0x00FF00FF);
            // cursor_x += 20;
            // kernel_print_int(binfo, programInterface, 0x00FF00FF);
            // cursor_y += 20;
        }
    }
}

static inline void pci_enumerate_all(pciDeviceInfo* deviceInfo) {
    cursor_y = 190;
    cursor_x = 550;
    deviceInfo->foundDevice = 0;

    for (uint16_t i = 0; i < 256; i++) {
        pci_enumerate_bus(i, deviceInfo);
        if (deviceInfo->foundDevice) {
            break;
        }
    }

    uint32_t bar5 = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x24);
    uint32_t pciMemorySpaceDecoding = pci_read_config(deviceInfo->bus, deviceInfo->device, deviceInfo->function, 0x04) & 0xFFFF;

    // kernel_print_int(binfo, deviceInfo.bus, 0x00FF00FF);
    // cursor_x += 20;
    // kernel_print_int(binfo, deviceInfo.device, 0x00FF00FF);
    // cursor_x += 20;
    // kernel_print_int(binfo, deviceInfo.function, 0x00FF00FF);
    // cursor_x += 20;
    // kernel_print_hex(binfo, bar5, 0x00FF00FF);
    // cursor_x += 80;
    // kernel_print_int(binfo, pciMemorySpaceDecoding & 0x2, 0x00FF00FF);

    if (pciMemorySpaceDecoding & 0x2) {
        if (pciMemorySpaceDecoding & 0x4) {
            cursor_x += 20;
            // kernel_print(binfo, "Memory Space Decoding Enabled!", 0x00FF00FF);
        }
    }
}

#endif