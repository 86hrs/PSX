#include "gpu.h"

Gpu::Gpu() {
  page_base_x = 0;
  page_base_y = 0;

  semi_transparency = 0;

  texture_depth = TextureDepth::T4Bit;
  dithering = false;
  draw_to_display = false;
  force_set_mask_bit = false;
  preserve_masked_pixels = false;
  field = Field::Top;
  texture_disable = false;
  hres = HorizontalRes::from_fields(0, 0);
  vres = VerticalRes::Y240Lines;

  vmode = VMode::Ntsc;
  display_depth = DisplayDepth::D15Bits;

  interlaced = false;
  display_disabled = true;
  interrupt = false;
  dma_direction = DmaDirection::Off;
}

uint32_t Gpu::status() {
  uint32_t r = 0;

  r |= (uint32_t(this->page_base_x)) << 0;
  r |= (uint32_t(this->page_base_y)) << 4;
  r |= (uint32_t(this->semi_transparency)) << 5;
  r |= (uint32_t(this->texture_depth)) << 7;
  r |= (uint32_t(this->dithering)) << 9;
  r |= (uint32_t(this->draw_to_display)) << 10;
  r |= (uint32_t(this->force_set_mask_bit)) << 11;
  r |= (uint32_t(this->preserve_masked_pixels)) << 12;
  r |= (uint32_t(this->field)) << 13;
  r |= (uint32_t(this->texture_disable)) << 15;
  r |= this->hres.into_status();
  r |= (uint32_t(this->vres)) << 19;
  r |= (uint32_t(this->vmode)) << 20;
  r |= (uint32_t(this->display_depth)) << 21;
  r |= (uint32_t(this->interlaced)) << 22;
  r |= (uint32_t(this->display_disabled)) << 23;
  r |= (uint32_t(this->interrupt)) << 24;

  r |= 1 << 26;
  r |= 1 << 27;
  r |= 1 << 28;

  r |= (uint32_t(this->dma_direction)) << 29;
  r |= 0 << 31;

  uint32_t dma_request = 0;
  switch (this->dma_direction) {
  case DmaDirection::Off:
    dma_request = 0;
    break;
  case DmaDirection::Fifo:
    dma_request = 1;
    break;
  case DmaDirection::CpuToGp0:
    dma_request = (r >> 28) & 1;
    break;
  case DmaDirection::VRamToCpu:
    dma_request = (r >> 27) & 1;
    break;
  }
  r |= dma_request << 25;

  return r;
}
