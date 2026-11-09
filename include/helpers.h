#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

static inline void setByteAtIndex_uint8(volatile uint8_t* address, uint8_t index, uint8_t value) {
    address[index] = value;
}

static inline void setBitAtOffset_uint32(volatile uint32_t* address, int offset, uint32_t value) {
    _Bool shiftRight = 0;
    if (offset < 0) {
        offset *= -1;
        shiftRight = 1;
    }

    if (shiftRight) {
        *address |= (value >> offset);
    } else {
        *address |= (value << offset);
    }
}

static inline void setBitAtOffset_uint16(volatile uint16_t* address, int offset, uint16_t value) {
    _Bool shiftRight = 0;
    if (offset < 0) {
        offset *= -1;
        shiftRight = 1;
    }

    if (shiftRight) {
        *address |= (value >> offset);
    } else {
        *address |= (value << offset);
    }
}

static inline void setBitAtOffset_uint8(volatile uint8_t* address, int offset, uint8_t value) {
    _Bool shiftRight = 0;
    if (offset < 0) {
        offset *= -1;
        shiftRight = 1;
    }

    if (shiftRight) {
        *address |= (value >> offset);
    } else {
        *address |= (value << offset);
    }
}

static inline _Bool checkForTimeout_uint32(volatile uint32_t* address, uint32_t index, int offset, uint32_t bitMask, _Bool condition) {
    _Bool shiftRight = 0;
    if (offset < 0) {
        offset *= -1;
        shiftRight = 1;
    }

    uint32_t count = 0;
    if (shiftRight) {
        while (((address[index] >> offset) & bitMask) != condition) {
            count++;

            if (count >= 100000)
                return 0; // timeout
        }
    } else {
        while (((address[index] << offset) & bitMask) != condition) {
            count++;

            if (count >= 100000)
                return 0; // timeout
        }
    }

    return 1; // continue
}

static inline _Bool checkForTimeout_uint8(volatile uint8_t* address, uint8_t index, int offset, uint8_t bitMask, _Bool condition) {
    _Bool shiftRight = 0;
    if (offset < 0) {
        offset *= -1;
        shiftRight = 1;
    }

    uint32_t count = 0;
    if (shiftRight) {
        while (((address[index] >> offset) & bitMask) != condition) {
            count++;

            if (count >= 100000)
                return 0; // timeout
        }
    } else {
        while (((address[index] << offset) & bitMask) != condition) {
            count++;

            if (count >= 100000)
                return 0; // timeout
        }
    }

    return 1; // continue
}

static inline _Bool checkForTimeout_uint16(uint16_t* address, uint16_t index, int offset, uint16_t bitMask, _Bool condition) {
    _Bool shiftRight = 0;
    if (offset < 0) {
        offset *= -1;
        shiftRight = 1;
    }

    uint32_t count = 0;
    if (shiftRight) {
        while (((address[index] >> offset) & bitMask) != condition) {
            count++;

            if (count >= 100000)
                return 0; // timeout
        }
    } else {
        while (((address[index] << offset) & bitMask) != condition) {
            count++;

            if (count >= 100000)
                return 0; // timeout
        }
    }

    return 1; // continue
}

#endif