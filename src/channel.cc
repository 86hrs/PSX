#include "channel.h"
#include <cstdio>
#include <exception>
#include <optional>

Channel::Channel() {
    this->enable = false;
    this->direction = Direction::ToRam;
    this->step = Step::Increment;
    this->sync = Sync::Manual;
    this->trigger = false;
    this->chop = false;
    this->chop_dma_sz = 0;
    this->chop_cpu_sz = 0;
    this->dummy = 0;
    this->base = 0;
    this->block_count = 0;
    this->block_size = 0;
}

uint32_t Channel::get_control() {
    uint32_t r = 0;
    r |= (this->direction) << 0;
    r |= (this->step) << 1;
    r |= ((uint32_t)this->chop) << 8;
    r |= (this->sync) << 9;
    r |= ((uint32_t)this->chop_dma_sz) << 16;
    r |= ((uint32_t)this->chop_cpu_sz) << 20;
    r |= ((uint32_t)this->enable) << 24;
    r |= ((uint32_t)this->trigger) << 28;
    r |= (uint32_t(this->dummy)) << 29;

    return r;
}

void Channel::set_control(uint32_t p_val) {
    // bit 0: direction
    this->direction = (p_val & 1) ? Direction::FromRam
                                  : Direction::ToRam; // ok

    // bit 1: address step  (FIXED — was using bit 8)
    this->step = ((p_val >> 1) & 1) ? Step::Decrement
                                    : Step::Increment; // <-- fix

    // bit 8: chopping enable
    this->chop = ((p_val >> 8) & 1) != 0; // ok

    // bits 10..9: sync mode
    switch ((p_val >> 9) & 3) {
    case 0:
        this->sync = Sync::Manual;
        break;
    case 1:
        this->sync = Sync::Request;
        break;
    case 2:
        this->sync = Sync::LinkedList;
        break;
    default:
        printf("Unknown DMA sync mode\n");
        std::terminate();
    }

    // bits 18..16 and 22..20: chop sizes
    this->chop_dma_sz = uint8_t((p_val >> 16) & 7);
    this->chop_cpu_sz = uint8_t((p_val >> 20) & 7);

    // bit 24: enable, bit 28: trigger, bits 30..29: dummy
    this->enable = ((p_val >> 24) & 1) != 0;
    this->trigger = ((p_val >> 28) & 1) != 0;
    this->dummy = uint8_t((p_val >> 29) & 3);
}

uint32_t Channel::get_base() { return this->base; }

void Channel::set_base(uint32_t p_val) {
    this->base = p_val & 0xffffff;
}

uint32_t Channel::block_control() {
    uint32_t bs = (uint32_t)this->block_size;
    uint32_t bc = (uint32_t)this->block_count;

    return (bc << 16) | bs;
}

void Channel::set_block_control(uint32_t p_val) {
    this->block_size = (uint16_t)p_val;
    this->block_count = uint16_t(p_val >> 16);
}

bool Channel::active() {
    bool trg;
    if (this->sync == Sync::Manual)
        trg = this->trigger;
    else
        trg = true;

    return this->enable && trg;
}

Direction Channel::get_direction() { return this->direction; }

Step Channel::get_step() { return this->step; }

Sync Channel::get_sync() { return this->sync; }

void Channel::done() {
    this->enable = false;
    this->trigger = false;
}

std::optional<uint32_t> Channel::transfer_size() {
    switch (this->sync) {
    case Sync::Manual:
        return (uint32_t)(this->block_size);
    case Sync::Request:
        return (uint32_t)(this->block_size) * this->block_count;
    case Sync::LinkedList:
        return std::nullopt;
    }
    return std::nullopt;
}
