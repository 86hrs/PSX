#include "ram.h"
#include <iostream>

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

    std::cout << "RAM::store32: Wrote 0x" << std::hex << value
              << " to 0x" << offset << "\n";
}

uint8_t RAM::load8(uint32_t p_offset) {
    return this->data[p_offset];
}

void RAM::store8(uint32_t p_offset, uint8_t p_value) {
    this->data[p_offset] = p_value;
    std::cout << "RAM::store8: Wrote 0x" << p_value << " to 0x"
              << p_offset << "\n";
}
