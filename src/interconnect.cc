#include "interconnect.h"
#include "bios.h"
#include "map.h"
#include <cstdint>
#include <optional>

Interconnect::Interconnect(Bios *p_bios, RAM *p_ram, Dma *p_dma)
    : bios(p_bios), ram(p_ram), dma(p_dma){}

uint32_t Interconnect::mask_region(uint32_t p_addr) {
    uint8_t index = p_addr >> 29;
    uint32_t masked = p_addr & REGION_MASK[index];
    return masked;
}

void Interconnect::store16(uint32_t p_addr, uint16_t p_val) {
    (void)p_val;

    uint32_t addr = mask_region(p_addr);

    // IQR
    if (auto offset = map::IQR_CONTROL.contains(addr);
        offset.has_value()) {
        printf("Unhandled IQR to register: 0x%x\n", addr);
        return;
    }

    // SPU Registers
    if (auto offset = map::SPU.contains(addr);
        offset.has_value()) {
        printf("Unhandled store16 to SPU register: 0x%x\n",
               *offset);
        return;
    }

    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        return this->ram->store16(*offset, p_val);
    }

    printf("WARNING: Unhandled store16 to address: 0x%x\n",
           addr);
}

void Interconnect::store8(uint32_t p_addr, uint8_t p_val) {
    (void)p_val;

    uint32_t addr = mask_region(p_addr);

    // EXPANSION 2
    if (auto offset = map::EXPANSION_2.contains(addr);
        offset.has_value()) {
        printf("Unhandled store8 to EXPANSION2 register: 0x%x\n",
               *offset);
        return;
    }
    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        this->ram->store8(*offset, p_val);
        return;
    }

    printf("WARNING: Unhandled store8 to address: 0x%x\n", addr);
}

void Interconnect::store32(uint32_t p_addr, uint32_t p_val) {
    uint32_t addr = mask_region(p_addr);

    if (addr == 0x1f801060)
        return;               // RAM_SIZE (ignored)
    if (addr == 0xfffe0130) { // CACHE_CONTROL
        printf("CACHE_CONTROL write: 0x%x\n", p_val);
        return;
    }

    // INTERRUPT CONTROL REG
    if (auto offset = map::IQR_CONTROL.contains(p_addr);
        offset.has_value()) {
        printf("IQR write: 0x%x\n", p_val);
        return;
    }
    // GPU
    if (auto offset = map::GPU_GP0.contains(p_addr);
        offset.has_value()) {
        printf("GPU0 write: 0x%x\n", p_val);
        return;
    }
    // TIMER
    if (auto offset = map::TIMER_1.contains(p_addr);
        offset.has_value()) {
        printf("TIMER1 write: 0x%x\n", p_val);
        return;
    }
    if (auto offset = map::TIMER_2.contains(p_addr);
        offset.has_value()) {
        printf("TIMER2 write: 0x%x\n", p_val);
        return;
    }
    if (auto offset = map::TIMER_0.contains(p_addr);
        offset.has_value()) {
        printf("TIMER0 write: 0x%x\n", p_val);
        return;
    }
    // DMA
    if (auto offset = map::DMA.contains(p_addr);
        offset.has_value()) {
        printf("DMA store at: 0x%x: 0x%x\n", p_addr, *offset);
        return;
    }

    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        this->ram->store32(*offset, p_val);
        return;
    }

    // MEM_CONTROL
    if (auto offset = map::MEM_CONTROL.contains(addr);
        offset.has_value()) {
        switch (offset.value()) {
        case 0: // Expansion 1 base
            if (p_val != 0x1f000000) {
                printf("BAD expansion 1 base: 0x%x\n", p_val);
                std::terminate();
            }
            break;
        case 4: // Expansion 2 base
            if (p_val != 0x1f802000) {
                printf("BAD expansion 2 base: 0x%x\n", p_val);
                std::terminate();
            }
            break;
        default:
            printf(
                "Unhandled MEM_CONTROL write at offset: 0x%x\n",
                *offset);
            break;
        }
        return;
    }

    printf("WARNING: Unhandled store32 to address: 0x%x\n",
           addr);
}

uint32_t Interconnect::load32(uint32_t p_addr) {
    uint32_t addr = mask_region(p_addr);
    // GPU
    if (auto offset = map::GPU_GP1.contains(p_addr);
        offset.has_value()) {
        switch (*offset) {
        case 4:
            return (uint32_t)0x10000000;
        default:
            return 0x0;
        }
    }
    // DMA
    if (auto offset = map::DMA.contains(p_addr);
        offset.has_value()) {
        printf("DMA read at: 0x%x\n", p_addr);
        return 0x0;
    }

    // EXPANSION 1
    if (auto offset = map::EXPANSION_1.contains(p_addr);
        offset.has_value()) {
        printf("EXPANSION 1 read at: 0x%x\n", p_addr);
        return 0;
    }

    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        return ram->load32(*offset);
    }
    // BIOS
    if (auto offset = map::BIOS.contains(addr);
        offset.has_value()) {
        return bios->load32(*offset);
    }

    printf("WARNING: Unhandled load32 to address: 0x%x\n", addr);
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
    printf("WARNING: Unhandled load8 to address: 0x%x\n", addr);
    return 0xd8;
}
uint16_t Interconnect::load16(uint32_t p_addr) {
    uint32_t addr = mask_region(p_addr);

    // SPU
    if (auto offset = map::SPU.contains(addr);
        offset.has_value()) {
        printf("Unhandled load16 to SPU register: 0x%x\n", addr);
        return 0;
    }
    // IQR
    if (auto offset = map::IQR_CONTROL.contains(addr);
        offset.has_value()) {
        printf("Unhandled load16 to IQR register: 0x%x\n", addr);
        return 0;
    }

    // RAM
    if (auto offset = map::RAM.contains(addr);
        offset.has_value()) {
        return this->ram->load16(*offset);
    }
    printf("WARNING: Unhandled load16 to address: 0x%x\n", addr);

    return 0xd8;
}
