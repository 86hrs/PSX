#pragma once
#include <cstdint>

struct Dma {
    uint32_t control;
    // master IRQ enable
    bool irq_en;
    // IRQ enable for individual channels
    uint8_t channel_irq_en;
    // IRQ flags for individual channels
    uint8_t channel_irq_flags;
    // When set th einterrupt is active unconditionally (even if irq_en is false)
    bool force_irq;
    // Bits [0:5] of the interrupt registers are RW but I don't know
    // what they're supposed to do so I just store them and send them
    // back untouched on reads
    uint8_t irq_dummy;
   

    Dma();
    ~Dma() = default;

    uint32_t get_control();
    bool get_irq();
    uint32_t interrupt();
    void set_interrupt(uint32_t p_val);
    void set_control(uint32_t p_val);
};
