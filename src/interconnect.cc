#include "interconnect.h"
#include "bios.h"
#include "dma.h"
#include "map.h"
#include <cstdint>
#include <cstdio>
#include <exception>
#include <optional>

template void Interconnect::store<uint8_t>(uint32_t p_addr,
                                           uint8_t);
template void Interconnect::store<uint16_t>(uint32_t p_addr,
                                            uint16_t);
template void Interconnect::store<uint32_t>(uint32_t p_addr,
                                            uint32_t);

template uint8_t Interconnect::load<uint8_t>(uint32_t);
template uint16_t Interconnect::load<uint16_t>(uint32_t);
template uint32_t Interconnect::load<uint32_t>(uint32_t);

Interconnect::Interconnect(Bios *p_bios, RAM *p_ram, Dma *p_dma,
                           GPU *p_gpu)
    : bios(p_bios), ram(p_ram), dma(p_dma), gpu(p_gpu) {}

uint32_t Interconnect::mask_region(uint32_t p_addr) {
    uint8_t index = p_addr >> 29;
    uint32_t masked = p_addr & REGION_MASK[index];
    return masked;
}

void Interconnect::do_dma(Port p_port) {
    if (this->dma->get_mut_channel(p_port).get_sync() ==
        Sync::LinkedList) {
        this->do_dma_linked_list(p_port);
        return;
    }
    this->do_dma_block(p_port);
}

void Interconnect::do_dma_linked_list(Port p_port) {
    Channel &channel = this->dma->get_mut_channel(p_port);

    uint32_t addr = channel.get_base() & 0x1ffffc;

    if (channel.get_direction() == Direction::ToRam) {
        printf("Inavlid DMA direction for linked list mode\n");
        std::terminate();
    }

    if (p_port != Port::Gpu) {
        printf("Attempted linked list DMA on port: %d\n",
               p_port);
        std::terminate();
    }
    for (;;) {
        uint32_t header = this->ram->load<uint32_t>(addr);
        uint32_t remsz = header >> 24;

        while (remsz > 0) {
            addr = (addr + 4) & 0x1ffffc;
            uint32_t command = this->ram->load<uint32_t>(addr);
            this->gpu->gp0(command);
            remsz -= 1;
        }
        if ((header & 0x800000) != 0) {
            break;
        }
        addr = header & 0x1ffffc;
    }
    channel.done();
}

void Interconnect::do_dma_block(Port p_port) {
    Channel &channel = this->dma->get_mut_channel(p_port);

    int32_t increment = 0;

    if (channel.get_step() == Step::Increment) {
        increment = 4;
    } else if (channel.get_step() == Step::Decrement) {
        increment = -4;
    } else {
        printf("Invalid DMA step mode\n");
        std::terminate();
    }

    uint32_t addr = channel.get_base();

    uint32_t remsz;
    if (auto transfer_size = channel.transfer_size();
        transfer_size.has_value()) {
        remsz = *transfer_size;
    } else {
        printf("Couldn't figure out DMA block transfer size");
        std::terminate();
    }

    while (remsz > 0) {
        // Address wrapping logic, hardware may ignore LSBs
        uint32_t cur_addr = addr & 0x1FFFFC;

        switch (channel.get_direction()) {
        case Direction::FromRam: {
            uint32_t src_word =
                this->ram->load<uint32_t>(cur_addr);
            if (p_port == Port::Gpu) {
                this->gpu->gp0(src_word);
            } else {
                printf("Unhandled DMA destination port: %d\n",
                       p_port);
                std::terminate();
            }
            break;
        }
        case Direction::ToRam: {
            uint32_t src_word = 0;

            switch (p_port) {
            case Port::Otc:
                if (remsz == 1)
                    // Last word: end of ordering table
                    // marker
                    src_word = 0x00FFFFFF;
                else
                    // Otherwise: pointer to previous entry
                    src_word = (addr - 4) & 0x1FFFFF;
                break;
            default:
                printf("ERROR: Unhandled DMA source port %d\n",
                       (uint8_t)p_port);
                std::exit(1);
            }

            this->ram->store<uint32_t>(cur_addr, src_word);
            break;
        }
        default:
            printf("ERROR: Unknown DMA direction");
            std::exit(1);
        }

        addr += increment;
        remsz--;
    }
    channel.done();
}

uint32_t Interconnect::dma_reg(uint32_t p_offset) {
    uint32_t major = (p_offset & 0x70) >> 4;
    uint32_t minor = p_offset & 0xf;

    switch (major) {
    case 0 ... 6: {
        Port major_port = (Port)major;
        Channel &channel =
            this->dma->get_mut_channel(major_port);
        if (minor == 8) {
            return channel.get_control();
        } else {
            printf("Unhandled DMA read at: 0x%x\n", p_offset);
            std::terminate();
        }
    }
    case 7: {
        if (minor == 0) {
            return this->dma->get_control();
        } else if (minor == 4) {
            return this->dma->interrupt();
        } else {
            printf("Unhandled DMA read at: 0x%x\n", p_offset);
            std::terminate();
        }
    }
    default:
        printf("Unhandled DMA read at: 0x%x\n", p_offset);
        std::terminate();
    }
}
void Interconnect::set_dma_reg(uint32_t p_offset,
                               uint32_t p_val) {
    uint32_t major = (p_offset & 0x70) >> 4;
    uint32_t minor = p_offset & 0xf;

    std::optional<Port> active_port;

    switch (major) {
    case 0 ... 6: {
        Port port = (Port)major;
        Channel &channel = this->dma->get_mut_channel(port);
        switch (minor) {
        case 0x0:
            channel.set_base(p_val);
            break;
        case 0x4:
            channel.set_block_control(p_val);
            break;
        case 0x8:
            channel.set_control(p_val);
            break;
        default:
            printf("Unhandled DMA write at: 0x%x, val: "
                   "0x%08x, major: %d, minor: %d\n",
                   p_offset, p_val, major, minor);
            std::terminate();
        }

        if (channel.active()) {
            active_port = port;
        }
        break;
    }
    case 7: {
        if (minor == 0) {
            this->dma->set_control(p_val);
            break;
        }
        if (minor == 4) {
            this->dma->set_interrupt(p_val);
            break;
        }
    }
    default:
        printf("Unhandled DMA write at: 0x%x, val: 0x%08x, "
               "major: %d, minor: %d\n",
               p_offset, p_val, major, minor);
        std::terminate();
    }
    if (active_port.has_value()) {
        this->do_dma(*active_port);
    }
}

template <class T>
void Interconnect::store(uint32_t p_addr, T p_val) {
    uint32_t addr = mask_region(p_addr);
    if constexpr (sizeof(T) == 4) {
        if (addr == 0x1f801060)
            return;               // RAM_SIZE (ignored)
        if (addr == 0xfffe0130) { // CACHE_CONTROL
            printf("CACHE_CONTROL write: 0x%x\n", p_val);
            return;
        }
        // INTERRUPT CONTROL REG
        if (auto offset = map::IRQ_CONTROL.contains(p_addr);
            offset.has_value()) {
            switch (*offset) {
            case 0: // I_STAT - write 1 to clear bits
                irq_status &=
                    ~p_val; // Clear the bits that are set to 1
                printf("I_STAT write: 0x%08x (new status: "
                       "0x%08x)\n",
                       p_val, irq_status);
                return;
            case 4: // I_MASK - set interrupt mask
                irq_mask = p_val;
                printf("I_MASK write: 0x%08x\n", p_val);
                return;
            default:
                printf("Unhandled IRQ write at offset: 0x%x, "
                       "val: 0x%08x\n",
                       *offset, p_val);
                return;
            }
            return;
        }
        // GPU
        if (auto offset = map::GPU_GP0.contains(p_addr);
            offset.has_value()) {
            this->gpu->gp0(p_val);
            return;
        }
        if (auto offset = map::GPU_GP1.contains(p_addr);
            offset.has_value()) {
            this->gpu->gp1(p_val);
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
            printf("DMA write: 0x%x\n", p_val);
            this->set_dma_reg(*offset, p_val);
            return;
        }

        // RAM
        if (auto offset = map::RAM.contains(addr);
            offset.has_value()) {
            this->ram->store<uint32_t>(*offset, p_val);
            return;
        }

        // MEM_CONTROL
        if (auto offset = map::MEM_CONTROL.contains(addr);
            offset.has_value()) {
            switch (offset.value()) {
            case 0: // Expansion 1 base
                if (p_val != 0x1f000000) {
                    printf("BAD expansion 1 base: 0x%x\n",
                           p_val);
                    std::terminate();
                }
                break;
            case 4: // Expansion 2 base
                if (p_val != 0x1f802000) {
                    printf("BAD expansion 2 base: 0x%x\n",
                           p_val);
                    std::terminate();
                }
                break;
            default:
                printf("Unhandled MEM_CONTROL write at offset: "
                       "0x%x\n",
                       *offset);
                break;
            }
            return;
        }

        printf("WARNING: Unhandled store32 to address: 0x%x\n",
               addr);
    }

    if constexpr (sizeof(T) == 2) {
        // IQR
        if (auto offset = map::IRQ_CONTROL.contains(addr);
            offset.has_value()) {
            printf("Unhandled store16 to IQR register: 0x%x\n",
                   addr);
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
            return this->ram->store<uint16_t>(*offset, p_val);
        }

        printf("WARNING: Unhandled store16 to address: 0x%x\n",
               addr);
    }

    if constexpr (sizeof(T) == 1) {
        // EXPANSION 2
        if (auto offset = map::EXPANSION_2.contains(addr);
            offset.has_value()) {
            printf("Unhandled store8 to EXPANSION2 register: "
                   "0x%x\n",
                   *offset);
            return;
        }
        // RAM
        if (auto offset = map::RAM.contains(addr);
            offset.has_value()) {
            this->ram->store<uint8_t>(*offset, p_val);
            return;
        }
        // CDROM
        if (auto offset = map::CDROM.contains(addr);
            offset.has_value()) {
            printf("Unhandled store8 to CDROM register: "
                   "0x%x\n",
                   *offset);
            return;
        }

        printf("WARNING: Unhandled store8 to address: 0x%x\n",
               addr);
    }
}

template <typename T> T Interconnect::load(uint32_t p_addr) {
    fflush(stdout);
    uint32_t addr = mask_region(p_addr);

    // Handle 32-bit specific cases
    if constexpr (sizeof(T) == 4) {
        // GPU
        if (auto offset = map::GPU_GP1.contains(p_addr);
            offset.has_value()) {
            return 0x1c000000;
        }
        if (auto offset = map::JOY_RX_DATA.contains(p_addr);
            offset.has_value()) {
            return 0;
        }
        if (auto offset = map::GPU_GP0.contains(p_addr);
            offset.has_value()) {
            this->gpu->read();
        }
        // DMA
        if (auto offset = map::DMA.contains(p_addr);
            offset.has_value()) {
            return this->dma_reg(*offset);
        }
        // EXPANSION 1
        if (auto offset = map::EXPANSION_1.contains(p_addr);
            offset.has_value()) {
            printf("EXPANSION 1 read at: 0x%x\n", p_addr);
            return 0;
        }
        // IRQ
        if (auto offset = map::IRQ_CONTROL.contains(addr);
            offset.has_value()) {
            switch (*offset) {
            case 0: // I_STAT - interrupt status
                printf("I_STAT read: 0x%08x\n", irq_status);
                return irq_status;
            case 4: // I_MASK - interrupt mask
                printf("I_MASK read: 0x%08x\n", irq_mask);
                return irq_mask;
            default:
                printf("Unhandled IRQ read at offset: 0x%x\n",
                       *offset);
                return 0;
            }
        }
        // BIOS
        if (auto offset = map::BIOS.contains(addr);
            offset.has_value()) {
            return bios->load<uint32_t>(*offset);
        }
        // RAM
        if (auto offset = map::RAM.contains(addr);
            offset.has_value()) {
            return ram->load<uint32_t>(*offset);
        }
        printf("WARNING: Unhandled load32 to address: 0x%x\n",
               addr);
        return 0xdeadbeef;
    }

    // Handle 16-bit specific cases
    if constexpr (sizeof(T) == 2) {
        // SPU
        if (auto offset = map::SPU.contains(addr);
            offset.has_value()) {
            printf("Unhandled load16 to SPU register: 0x%x\n",
                   addr);
            return 0;
        }
        // IQR
        if (auto offset = map::IRQ_CONTROL.contains(addr);
            offset.has_value()) {
            printf("Unhandled load16 to IQR register: 0x%x\n",
                   addr);
            return 0;
        }
        // RAM
        if (auto offset = map::RAM.contains(addr);
            offset.has_value()) {
            return this->ram->load<uint16_t>(*offset);
        }
        printf("WARNING: Unhandled load16 to address: 0x%x\n",
               addr);
        return 0xd8;
    }

    // Handle 8-bit specific cases
    if constexpr (sizeof(T) == 1) {
        // BIOS
        if (auto offset = map::BIOS.contains(addr);
            offset.has_value()) {
            return this->bios->load<uint8_t>(*offset);
        }
        // RAM
        if (auto offset = map::RAM.contains(addr);
            offset.has_value()) {
            return this->ram->load<uint8_t>(*offset);
        }
        // CDROM
        if (auto offset = map::CDROM.contains(addr);
            offset.has_value()) {
            printf("Unhandled load8 to CDROM register: 0x%x\n",
                   *offset);
            return 0xd8;
        }
        printf("WARNING: Unhandled load8 to address: 0x%x\n",
               addr);
        return 0xd8;
    }

    static_assert(sizeof(T) == 1 || sizeof(T) == 2 ||
                      sizeof(T) == 4,
                  "Unsupported load size");
    return T{};
}

