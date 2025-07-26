#include "interconnect.h"
#include "bios.h"
#include "map.h"
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
    (void)p_val;

    uint32_t addr = mask_region(p_addr);

    // SPU Registers
    if (auto offset = map::SPU.contains(addr);
        offset.has_value()) {
        std::cout << "Unhandled store16 to SPU register: 0x"
                  << std::hex << offset.value() << "\n";
        return;
    }

    std::cout << "WARNING: Unhandled store16 to address: 0x"
              << std::hex << addr << "\n";
}

void Interconnect::store8(uint32_t p_addr, uint8_t p_val) {
    (void)p_val;

    uint32_t addr = mask_region(p_addr);

    // EXPANSION 2
    if (auto offset = map::EXPANSION_2.contains(addr);
        offset.has_value()) {
        std::cout
            << "Unhandled store8 to EXPANSION 2 register: 0x"
            << std::hex << offset.value() << "\n";
        return;
    }
    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        this->ram->store8(offset.value(), p_val);
        return;
    }

    std::cout << "WARNING: Unhandled store8 to address 0x"
              << std::hex << addr << "\n";
}

void Interconnect::store32(uint32_t p_addr, uint32_t p_val) {
    uint32_t addr = mask_region(p_addr);

    if (addr == 0x1f801060)
        return;               // RAM_SIZE (ignored)
    if (addr == 0xfffe0130) { // CACHE_CONTROL
        std::cout << "CACHE_CONTROL write: 0x" << std::hex
                  << p_val << "\n";
        return;
    }

    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        this->ram->store32(offset.value(), p_val);
        return;
    }

    // MEM_CONTROL
    if (auto offset = map::MEM_CONTROL.contains(addr);
        offset.has_value()) {
        switch (offset.value()) {
        case 0: // Expansion 1 base
            if (p_val != 0x1f000000) {
                std::cout << "Bad expansion 1 base: 0x"
                          << std::hex << p_val << "\n";
                std::terminate();
            }
            break;
        case 4: // Expansion 2 base
            if (p_val != 0x1f802000) {
                std::cout << "Bad expansion 2 base: 0x"
                          << std::hex << p_val << "\n";
                std::terminate();
            }
            break;
        default:
            std::cout
                << "Unhandled MEM_CONTROL write at offset: 0x"
                << std::hex << offset.value() << "\n";
            break;
        }
        return;
    }

    std::cout << "Unhandled store32 to address: 0x" << std::hex
              << addr << "\n";
}

uint32_t Interconnect::load32(uint32_t p_addr) {
    uint32_t addr = mask_region(p_addr);
    // EXPANSION 1
    if (auto offset = map::EXPANSION_1.contains(p_addr);
        offset.has_value()) {
        return 0xdeadbeef;
    }

    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        return ram->load32(offset.value());
    }
    // BIOS
    if (auto offset = map::BIOS.contains(addr);
        offset.has_value()) {
        return bios->load32(offset.value());
    }

    std::cout << "Unhandled load32 from address: 0x" << std::hex
              << addr << "\n";
    return 0xdeadbeef;
}

uint8_t Interconnect::load8(uint32_t p_addr) {
    uint32_t addr = mask_region(p_addr);

    // BIOS
    if (auto offset = map::BIOS.contains(addr);
        offset.has_value()) {
        return this->bios->load8(*offset);
    }
    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        return this->ram->load8(*offset);
    }
    std::cout << "Unhandled load8 from address: 0x" << std::hex
              << addr << "\n";

    return 0xd8;
}
