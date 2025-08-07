#pragma once
#include <cstdint>
#include "channel.h"

enum Port : uint32_t {
    /// Macroblock decoder input
    MdecIn = 0,
    /// Macroblock decoder output
    MdecOut = 1,
    /// Graphics Processing Unit
    Gpu = 2,
    /// CD-ROM drive
    CdRom = 3,
    /// Sound Processing Unit
    Spu = 4,
    /// Extension port
    Pio = 5,
    /// Used to clear the ordering table
    Otc = 6
};

struct Dma {
    uint32_t control;
    // master IRQ enable
    bool irq_en;
    // IRQ enable for individual channels
    uint8_t channel_irq_en;
    // IRQ flags for individual channels
    uint8_t channel_irq_flags;
    // When set th einterrupt is active unconditionally (even if
    // irq_en is false)
    bool force_irq;
    // Bits [0:5] of the interrupt registers are RW but I don't
    // know what they're supposed to do so I just store them and
    // send them back untouched on reads
    uint8_t irq_dummy;

    // 7 channel instances
    Channel channels[7];

    const Channel &get_channel(Port p_port);
    Channel &get_mut_channel(Port p_port);

    Dma();
    ~Dma() = default;

    uint32_t get_control();
    bool get_irq();
    uint32_t interrupt();
    void set_interrupt(uint32_t p_val);
    void set_control(uint32_t p_val);
};
