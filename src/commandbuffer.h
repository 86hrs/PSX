#pragma once
#include <cstdint>

struct CommmandBuffer {
    public:
    uint32_t buffer[12];
    uint8_t length;

    CommmandBuffer();
    ~CommmandBuffer() = default;

    void clear();
    void push_word(uint32_t p_word);
    uint32_t& operator[](uint32_t p_index);
};
