
#include "interconnect.h"
#include "bios.h"
#include "map.h"
#include <cassert>
#include <iostream>
#include <optional>

Interconnect::Interconnect(Bios *p_bios) { this->bios = p_bios; }

void Interconnect::store32(uint32_t p_addr, uint32_t p_val) {
    uint32_t addr = p_addr;
    assert(addr % 4 == 0 && "Unaligned store32 address");

    std::optional<uint32_t> offset = map::MEM_CONTROL.contains(addr);

    if (offset.has_value()) {
        switch (offset.value()) {
        case 0: {
            if (p_val != 0x1f000000) {
                std::cerr << "ERROR: Bad expansion 1 base address << " << p_val
                          << "\n";
                std::terminate();
            }
            break;
        }
        case 4: {
            if (p_val != 0x1f802000) {
                std::cerr << "ERROR: Bad expansion 2 base address << " << p_val
                          << "\n";
                std::terminate();
            }
            break;
        }
        default: {
            std::cerr << "WARNING: Unhandled write to MEM CONTROL\n";
            break;
        }
        }
        return;
    }
    std::cerr << "ERROR: Unhandled store32 into address << " << std::hex << addr
              << "\n";
    std::terminate();
}
uint32_t Interconnect::load32(uint32_t p_addr) {
    uint32_t addr = p_addr;
    assert(addr % 4 == 0 && "ERROR: Unaligned load32 address");

    if (p_addr >= 0x9FC00000 && p_addr < 0x9FC80000) {
        addr = 0xBFC00000 + (addr - 0x9FC00000);
    }
    std::optional<uint32_t> test = map::BIOS.contains(0xbfc00000);
    assert(test.has_value() && test.value() == 0);

    std::optional<uint32_t> offset = map::BIOS.contains(addr);

    if (offset.has_value())
        return this->bios->load32(offset.value());
    else
        std::cerr << "ERROR: Unhandled fetch32 at address << " << std::hex << p_addr
                  << "\n";
    return 0xdeadbeef;
}
