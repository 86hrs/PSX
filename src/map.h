#pragma once
#include <cstdint>
#include <optional>

namespace map {

struct Range {
    uint32_t start;
    uint32_t length;

    Range(uint32_t p_start, uint32_t p_length) {
        this->start = p_start;
        this->length = p_length;
    }

    std::optional<uint32_t> contains(uint32_t addr) const {
        if (addr >= start and addr < start + length) {
            return addr - start;
        }
        return std::nullopt;
    }
};

// Main memory and system regions
const Range BIOS(0x1fc00000, 512 * 1024);
const Range RAM(0x00000000, 2 * 1024 * 1024);
const Range RAM_SIZE(0x1f801060, 4);
const Range MEM_CONTROL(0x1f801000, 36);
const Range CACHE_CONTROL(0xfffe0130, 4);

// I/O devices
const Range DMA(0x1f801080, 0x80);
const Range IQR_CONTROL(0x1f801070, 8);
const Range GPU_GP0(0x1f801810, 4);       // GPU command port
const Range GPU_GP1(0x1f801814, 4);       // GPU control port
const Range GPU_STATUS(0x1f801820, 4);    // GPU status (read mirror)
const Range SPU(0x1f801c00, 640);

// Timer registers (3 general-purpose timers)
const Range TIMER_0(0x1f801100, 0x10);
const Range TIMER_1(0x1f801110, 0x10);
const Range TIMER_2(0x1f801120, 0x10);

// Expansion regions
const Range EXPANSION_1(0x1f000084, 1);
const Range EXPANSION_2(0x1f802000, 66);

// JOYPAD
const Range JOY_RX_DATA(0x1f801110, 1);
} // namespace map
