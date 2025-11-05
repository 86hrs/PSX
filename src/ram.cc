#include "ram.h"
#include <cstdint>
#include <cstring>

RAM::RAM() {
    memset(this->data, 0, sizeof(data));
}

template <typename T> T RAM::load(uint32_t p_offset) {
    switch (sizeof(T)) {
    case 1:
        return this->data[p_offset];
    case 2: {
        uint16_t b0 = (uint16_t)this->data[p_offset + 0];
        uint16_t b1 = (uint16_t)this->data[p_offset + 1];

        return b0 | (b1 << 8);
    }
    case 4: {
        uint64_t offset = (uint64_t)p_offset;

        uint32_t b0 = this->data[offset + 0];
        uint32_t b1 = this->data[offset + 1];
        uint32_t b2 = this->data[offset + 2];
        uint32_t b3 = this->data[offset + 3];

        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }
    }
}

template <typename T>
void RAM::store(uint32_t p_offset, T p_value) {
    switch (sizeof(T)) {
    case 1:
        this->data[p_offset] = p_value;
        return;
    case 2: {
        uint8_t b0 = (uint8_t)p_value;
        uint8_t b1 = (uint8_t)(p_value >> 8);

        this->data[p_offset + 0] = b0;
        this->data[p_offset + 1] = b1;
        return;
    }
    case 4: {
        // Split into bytes (little-endian)
        this->data[p_offset + 0] = p_value & 0xFF; // LSB
        this->data[p_offset + 1] = (p_value >> 8) & 0xFF;
        this->data[p_offset + 2] = (p_value >> 16) & 0xFF;
        this->data[p_offset + 3] = (p_value >> 24) & 0xFF; // MSB
        return;
    }
    }
}

template uint8_t RAM::load<uint8_t>(uint32_t);
template uint16_t RAM::load<uint16_t>(uint32_t);
template uint32_t RAM::load<uint32_t>(uint32_t);

template void RAM::store<uint8_t>(uint32_t, uint8_t);
template void RAM::store<uint16_t>(uint32_t, uint16_t);
template void RAM::store<uint32_t>(uint32_t, uint32_t);
