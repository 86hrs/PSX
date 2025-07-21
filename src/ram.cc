#include "ram.h"


RAM::RAM() {
    this->data.resize(2 * 1024 * 1024);    
    for(auto& d : data)
        d = 0xca;
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
    uint8_t b0 = value;
    uint8_t b1 = (uint8_t)(value >> 8);
    uint8_t b2 = (uint8_t)(value >> 16);
    uint8_t b3 = (uint8_t)(value >> 24);
    this->data[offset + 0] = b0;
    this->data[offset + 1] = b1;
    this->data[offset + 2] = b2;
    this->data[offset + 3] = b3;
}
