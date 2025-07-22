#include "interconnect.h"
#include "bios.h"
#include "map.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>

Interconnect::Interconnect(Bios *p_bios, RAM *p_ram)
    : bios(p_bios), ram(p_ram) {}

uint32_t Interconnect::mask_region(uint32_t p_addr) {
    uint8_t index = p_addr >> 29;
    uint32_t masked = p_addr & REGION_MASK[index];
    return masked;
}

void Interconnect::store16(uint32_t p_addr, uint16_t p_val) {
    uint32_t addr = mask_region(p_addr);
    assert(addr % 2 == 0 && "Unaligned store16 address");

    // TODO: Implement actual 16-bit store handling
    std::cerr << "Unhandled store16 to address 0x" << std::hex << addr << "\n";
}

void Interconnect::store32(uint32_t p_addr, uint32_t p_val) {
    uint32_t addr = mask_region(p_addr);
    assert(addr % 4 == 0 && "Unaligned store32 address");

    if (addr == 0x1f801060)
        return;               // RAM_SIZE (ignored)
    if (addr == 0xfffe0130) { // CACHE_CONTROL
        std::cout << "CACHE_CONTROL write: 0x" << std::hex << p_val << "\n";
        return;
    }

    // RAM
    if (auto offset = map::RAM.contains(addr); offset.has_value()) {
        this->ram->store32(offset.value(), p_val);
        return;
    }
    if (auto offset = map::RAM_KSEG0.contains(addr); offset.has_value()) {
        this->ram->store32(offset.value() & 0x1FFFFFFF,
                           p_val); // Strip KSEG0 bit
        return;
    }
    if (auto offset = map::RAM_KSEG1.contains(addr); offset.has_value()) {
        this->ram->store32(offset.value() & 0x1FFFFFFF,
                           p_val); // Strip KSEG1 bit
        return;
    }

    // MEM_CONTROL
    if (auto offset = map::MEM_CONTROL.contains(addr); offset.has_value()) {
        switch (offset.value()) {
        case 0: // Expansion 1 base
            if (p_val != 0x1f000000) {
                std::cerr << "Bad expansion 1 base: 0x" << std::hex << p_val
                          << "\n";
                std::terminate();
            }
            break;
        case 4: // Expansion 2 base
            if (p_val != 0x1f802000) {
                std::cerr << "Bad expansion 2 base: 0x" << std::hex << p_val
                          << "\n";
                std::terminate();
            }
            break;
        default:
            std::cerr << "Unhandled MEM_CONTROL write at offset 0x" << std::hex
                      << offset.value() << "\n";
            break;
        }
        return;
    }

    std::cout << "Unhandled store32 to address 0x" << std::hex << addr << "\n";
}

uint32_t Interconnect::load32(uint32_t p_addr) {
    uint32_t addr = mask_region(p_addr);

    assert(addr % 4 == 0 && "Unaligned load32 address");
    // RAM
    if (auto offset = map::RAM.contains(addr); offset.has_value()) {
        return ram->load32(offset.value());
    }
    // BIOS
    if (auto offset = map::BIOS.contains(addr); offset.has_value()) {
        return bios->load32(offset.value());
    }

    std::cerr << "Unhandled load32 from address 0x" << std::hex << addr << "\n";
    return 0xdeadbeef;
}
