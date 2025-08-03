#include "ram.h"
#include <cstdint>

RAM::RAM() {
    for (auto &d : data) {
        d = 0x0;
    }
}

uint32_t RAM::load32(uint32_t p_offset) {
    uint64_t offset = (uint64_t)p_offset;

    uint32_t b0 = this->data[offset + 0];
    uint32_t b1 = this->data[offset + 1];
    uint32_t b2 = this->data[offset + 2];
    uint32_t b3 = this->data[offset + 3];

    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

void RAM::store32(uint32_t offset, uint32_t value) {
    // Split into bytes (little-endian)
    this->data[offset + 0] = value & 0xFF; // LSB
    this->data[offset + 1] = (value >> 8) & 0xFF;
    this->data[offset + 2] = (value >> 16) & 0xFF;
    this->data[offset + 3] = (value >> 24) & 0xFF; // MSB
}

uint8_t RAM::load8(uint32_t p_offset) {
    return this->data[p_offset];
}

void RAM::store8(uint32_t p_offset, uint8_t p_value) {
    this->data[p_offset] = p_value;
}

void RAM::store16(uint32_t p_offset, uint16_t p_value) {
    uint8_t b0 = (uint8_t)p_value;
    uint8_t b1 = (uint8_t)(p_value >> 8);

    this->data[p_offset + 0] = b0;
    this->data[p_offset + 1] = b1;
}

uint16_t RAM::load16(uint32_t p_offset) {
    uint16_t b0 = (uint16_t)this->data[p_offset + 0];
    uint16_t b1 = (uint16_t)this->data[p_offset + 1];

    return b0 | (b1 << 8);
}
