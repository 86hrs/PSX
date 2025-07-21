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

// const Range BIOS(0xbfc00000, 512 * 1024);
// const Range RAM_SIZE(0x1f801060, 4);
// const Range RAM(0xa0000000, 2 * 1024 * 1024);
// const Range CACHE_CONTROL(0xfffe0130, 4);

const Range BIOS(0x1fc00000, 512 * 1024);
const Range RAM_SIZE(0x1f801060, 4);
const Range RAM(0x00000000, 2 * 1024 * 1024);
const Range MEM_CONTROL(0x1f801000, 36);
const Range CACHE_CONTROL(0xfffe0130, 4);
} // namespace map
