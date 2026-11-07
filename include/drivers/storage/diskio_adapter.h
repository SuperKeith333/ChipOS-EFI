#ifndef DISKIO_ADAPTER_H
#define DISKIO_ADAPTER_H

#include "drivers/storage/block_device.h"

void diskio_set_device(BlockDevice* blockDevice);

#endif