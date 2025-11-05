#include "dma.h"

Dma::Dma() { this->control = 0x07654321; }

uint32_t Dma::get_control() { return this->control; }
void Dma::set_control(uint32_t p_val) { this->control = p_val; }

bool Dma::get_irq() {
    bool channel_irq =
        this->channel_irq_flags & this->channel_irq_en;
    return this->force_irq || (this->irq_en && channel_irq != 0);
}

uint32_t Dma::interrupt() {
    uint32_t r = 0;

    r |= (uint32_t)this->irq_dummy;
    r |= ((uint32_t)this->force_irq) << 15;
    r |= ((uint32_t)this->channel_irq_en) << 16;
    r |= ((uint32_t)this->irq_en) << 23;
    r |= ((uint32_t)this->channel_irq_flags) << 24;
    r |= ((uint32_t)this->get_irq()) << 31;

    return r;
}

void Dma::set_interrupt(uint32_t p_val) {
    this->irq_dummy = (uint8_t)(p_val & 0x3f);
    this->force_irq = ((p_val >> 15) & 1) != 0;
    this->channel_irq_en = uint8_t((p_val >> 16) & 0x7f);
    this->irq_en = ((p_val >> 23) & 1);

    uint8_t ack = (uint8_t)((p_val >> 24) & 0x3f);
    this->channel_irq_flags &= ~ack;
}

const Channel& Dma::get_channel(Port p_port) {
    const Channel& ref = this->channels[p_port];
    return ref;
}
Channel& Dma::get_mut_channel(Port p_port) {
    Channel& ref = this->channels[p_port];
    return ref;
}
