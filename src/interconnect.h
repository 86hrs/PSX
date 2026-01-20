#pragma once
#include "bios.h"
#include "ram.h"
#include "dma.h"
#include "gpu.h"

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
    GPU *gpu;

    uint32_t irq_status = 0x0;
    uint32_t irq_mask = 0x0;

    Interconnect(Bios *, RAM *, Dma *, GPU *);
    ~Interconnect() = default;

    template <class T> T load(uint32_t p_addr);
    template <class V> void store(uint32_t p_addr, V);

    uint32_t mask_region(uint32_t p_addr);

    uint32_t dma_reg(uint32_t p_offset);
    void set_dma_reg(uint32_t p_offset, uint32_t p_val);

    void do_dma(Port);
    void do_dma_block(Port);
    void do_dma_linked_list(Port);
};
