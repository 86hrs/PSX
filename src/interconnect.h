#pragma once
#include "bios.h"
#include "ram.h"
#include "dma.h"

struct Interconnect {
    static constexpr uint32_t REGION_MASK[] = {
        // KUSEG: 2048MB
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        // KSEG0:  512MB
        0x7fffffff,
        // KSEG1:  512MB
        0x1fffffff,
        // KSEG2: 1024MB
        0xffffffff,
        0xffffffff,
    };

    Bios *bios;
    RAM *ram;
    Dma *dma;

    Interconnect(Bios *, RAM *, Dma *);
    ~Interconnect() = default;

    uint32_t load32(uint32_t);
    uint16_t load16(uint32_t);
    uint8_t load8(uint32_t);

    void store32(uint32_t, uint32_t);
    void store16(uint32_t, uint16_t);
    void store8(uint32_t, uint8_t);

    uint32_t mask_region(uint32_t p_addr);

    uint32_t dma_reg(uint32_t p_offset);
    void set_dma_reg(uint32_t p_offset, uint32_t p_val);

    void do_dma(Port);
    void do_dma_block(Port);
};
