#include <stddef.h>

#include "ff.h"
#include "diskio.h"
#include "drivers/storage/block_device.h"

static BlockDevice* _blockDevice = NULL;

void diskio_set_device(BlockDevice* blockDevice) {
    _blockDevice = blockDevice;
}

DSTATUS disk_initialize (BYTE pdrv) {
    if (pdrv != 0)
        return STA_NOINIT;
    if (_blockDevice == NULL)
        return STA_NOINIT;
    if (_blockDevice->read_sectors == NULL)
        return STA_NOINIT;
    if (_blockDevice->sector_size == 0)
        return STA_NOINIT;
    if (_blockDevice->sector_count == 0)
        return STA_NOINIT;
    
    return 0;
}

DSTATUS disk_status (BYTE pdrv) {
    if (pdrv != 0)
        return STA_NOINIT;
    if (_blockDevice->read_sectors == NULL)
        return STA_NOINIT;
    if (_blockDevice == NULL)
        return STA_NOINIT;
    if (_blockDevice->sector_size == 0)
        return STA_NOINIT;
    if (_blockDevice->sector_count == 0)
        return STA_NOINIT;
    
    return 0;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != 0)
        return RES_PARERR;
    if (_blockDevice == NULL)
        return RES_NOTRDY;
    if (_blockDevice->read_sectors == NULL)
        return RES_NOTRDY;
    
    BlockDeviceStatus _blockDeviceStatus = _blockDevice->read_sectors(_blockDevice->context, sector, count, buff);

    switch (_blockDeviceStatus) {
        case BLOCK_DEVICE_SUCCESS:
            return RES_OK;
            break;
        case BLOCK_DEVICE_DEVICE_ERROR:
            return RES_ERROR;
            break;
        case BLOCK_DEVICE_DEVICE_NOT_READY:
            return RES_NOTRDY;
            break;
        case BLOCK_DEVICE_INVALID_LBA:
            return RES_PARERR;
            break;
        case BLOCK_DEVICE_INVALID_ARGUMENT:
            return RES_PARERR;
            break;
        case BLOCK_DEVICE_TIMEOUT:
            return RES_ERROR;
            break;
        case BLOCK_DEVICE_IO_ERROR:
            return RES_ERROR;
            break;
        default:
            return RES_ERROR;
            break;
    }
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv != 0)
        return RES_PARERR;
    if (_blockDevice == NULL)
        return RES_NOTRDY;
    if (_blockDevice->read_sectors == NULL)
        return RES_NOTRDY;
    
    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *(LBA_t*)buff = _blockDevice->sector_count;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = _blockDevice->sector_size;
            return RES_OK;
        default:
            return RES_PARERR;
    }
}