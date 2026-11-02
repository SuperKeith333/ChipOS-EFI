#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include <stdint.h>

typedef enum {
    SUCCESS,
    DEVICE_ERROR,
    DEVICE_NOT_READY,
    INVALID_LBA,
    INVALID_ARGUMENT,
    TIMEOUT,
    IO_ERROR
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