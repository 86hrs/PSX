#include "dma.h"

Dma::Dma() { this->control = 0x07654321; }

uint32_t Dma::get_control() { return this->control; }
void Dma::set_control(uint32_t p_val) { this->control = p_val; }
