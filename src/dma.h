#pragma once
#include <cstdint>

struct Dma {
    uint32_t control;

    Dma();
    ~Dma() = default;

    uint32_t get_control();
    void set_control(uint32_t p_val);
};
