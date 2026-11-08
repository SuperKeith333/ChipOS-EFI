#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include <stdint.h>

typedef enum {
    BLOCK_DEVICE_SUCCESS,
    BLOCK_DEVICE_DEVICE_ERROR,
    BLOCK_DEVICE_DEVICE_NOT_READY,
    BLOCK_DEVICE_INVALID_LBA,
    BLOCK_DEVICE_INVALID_ARGUMENT,
    BLOCK_DEVICE_TIMEOUT,
    BLOCK_DEVICE_IO_ERROR
} BlockDeviceStatus;

typedef struct {
	uint32_t sector_size;
	uint64_t sector_count;

	BlockDeviceStatus (*read_sectors)(
		void *context,
		uint64_t lba,
		uint32_t count,
		void *buffer
	);

	void *context;
} BlockDevice;

#endif