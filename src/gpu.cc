#include "gpu.h"
#include "commandbuffer.h"
#include <cstdio>
#include <exception>

GPU::GPU(CommmandBuffer* p_commandbuffer) {
    this->page_base_x = 0;
    this->page_base_y = 0;
    this->semi_transparency = 0;
    this->texture_depth = TextureDepth::T4Bit;
    this->dithering = false;
    this->draw_to_display = false;
    this->force_set_mask_bit = false;
    this->preserve_masked_pixels = false;
    this->field = Field::Top;
    this->texture_disable = false;
    this->hres = HorizontalRes::from_fields(0, 0);
    this->vres = VerticalRes::Y240Lines;

    this->vmode = VMode::Ntsc;
    this->display_depth = DisplayDepth::D15Bits;

    this->interlaced = false;
    this->display_disabled = true;
    this->interrupt = false;
    this->dma_direction = DmaDirection::Off;

    this->texture_window_x_mask = 0;
    this->texture_window_y_mask = 0;
    this->texture_window_x_offset = 0;
    this->texture_window_y_offset = 0;
    this->drawing_area_left = 0;
    this->drawing_area_top = 0;
    this->drawing_area_right = 0;
    this->drawing_area_bottom = 0;
    this->drawing_x_offset = 0;
    this->drawing_y_offset = 0;
    this->display_vram_x_start = 0;
    this->display_vram_y_start = 0;
    this->display_horiz_start = 0x200;
    this->display_horiz_end = 0xc00;
    this->display_line_start = 0x10;
    this->display_line_end = 0x100;

    this->gp0_command_remaining = 0;
    this->gp0_command_ptr = nullptr;
    this->gp0_command = p_commandbuffer;

    this->gp0_mode = Gp0Mode::Command;
}

uint32_t GPU::status() {
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

void GPU::gp0(uint32_t p_val) {
    if (this->gp0_command_remaining == 0) {
        uint32_t opcode = (p_val >> 24) & 0xff;

        uint32_t len = 0;
        void (GPU::*method)(void) = nullptr;

        switch (opcode) {
        case 0xa0:
            len = 3;
            method = &GPU::gp0_image_load;
            break;
        case 0x01:
            len = 1;
            method = &GPU::gp0_clear_cache;
            break;
        case 0x28:
            len = 5;
            method = &GPU::gp0_quad_mono_opaque;
            break;
        // NOP
        case 0x0:
            len = 1;
            method = &GPU::gp0_nop;
            break;
        case 0xe1:
            len = 1;
            method = &GPU::gp0_draw_mode;
            break;
        case 0xe2:
            len = 1;
            method = &GPU::gp0_texture_window;
            break;
        case 0xe3:
            len = 1;
            method = &GPU::gp0_drawing_area_top_left;
            break;
        case 0xe4:
            len = 1;
            method = &GPU::gp0_drawing_area_bottom_right;
            break;
        case 0xe5:
            len = 1;
            method = &GPU::gp0_drawing_offset;
            break;
        case 0xe6:
            len = 1;
            method = &GPU::gp0_mask_bit_setting;
            break;
        default:
            printf("Unhandled GP0 command: 0x%x\n", p_val);
            std::terminate();
        }
        this->gp0_command_remaining = len;
        this->gp0_command_ptr = method;
        this->gp0_command->clear();
    }
    this->gp0_command_remaining -= 1;

    if (this->gp0_mode == Gp0Mode::Command) {
        this->gp0_command->push_word(p_val);
        if (this->gp0_command_remaining == 0)
            (this->*gp0_command_ptr)();
    } else if (this->gp0_mode == Gp0Mode::ImageLoad) {
        // XXX Should copy pixel data to VRAM
        if (this->gp0_command_remaining == 0)
            this->gp0_mode = Gp0Mode::Command;
    }
}
void GPU::gp1(uint32_t p_val) {
    uint32_t opcode = (p_val >> 24) & 0xff;

    switch (opcode) {
    case 0x0:
        this->gp1_reset(p_val);
        break;
    case 0x8:
        this->gp1_display_mode(p_val);
        break;
    case 0x4:
        this->gp1_dma_direction(p_val);
        break;
    case 0x5:
        this->gp1_display_vram_start(p_val);
        break;
    case 0x6:
        this->gp1_display_horizontal_range(p_val);
        break;
    case 0x7:
        this->gp1_display_vertical_range(p_val);
        break;
    default:
        printf("Unhandled GP1 command: 0x%x\n", p_val);
        std::terminate();
    }
}

void GPU::gp0_image_load() {
    // Parameters 2 contaains the image resolution
    uint32_t res = (*this->gp0_command)[2];

    uint16_t width = res & 0xffff;
    uint16_t height = res >> 16;

    // Size of the image in 16bit pixels
    uint16_t imgsize = width * height;

    // If we hae an odd number of pixels we must round up
    // since we transfer 32bits at a time. There'll be 16bits
    // of padding in the last word.
    imgsize = (imgsize + 1) & !1;

    // Store number of words expected for this image
    this->gp0_command_remaining = imgsize / 2;

    // Put the GP0 state machine in ImageLoad mode
    this->gp0_mode = Gp0Mode::ImageLoad;
}

void GPU::gp0_texture_window() {
    uint32_t p_val = (*this->gp0_command)[0];
    this->texture_window_x_mask = uint8_t(p_val & 0x1f);
    this->texture_window_y_mask = uint8_t((p_val >> 5) & 0x1f);
    this->texture_window_x_offset =
        uint8_t((p_val >> 10) & 0x1f);
    this->texture_window_y_offset =
        uint8_t((p_val >> 15) & 0x1f);
}

void GPU::gp0_drawing_area_top_left() {
    uint32_t p_val = (*this->gp0_command)[0];
    this->drawing_area_top = uint16_t((p_val >> 10) & 0x3ff);
    this->drawing_area_left = uint16_t(p_val & 0x3ff);
}

void GPU::gp0_drawing_area_bottom_right() {
    uint32_t p_val = (*this->gp0_command)[0];
    this->drawing_area_bottom = uint16_t((p_val >> 10) & 0x3ff);
    this->drawing_area_right = uint16_t(p_val & 0x3ff);
}

void GPU::gp0_drawing_offset() {
    uint32_t p_val = (*this->gp0_command)[0];
    uint16_t x = uint16_t(p_val & 0x7ff);
    uint16_t y = uint16_t((p_val >> 11) & 0x7ff);

    this->drawing_x_offset = (int16_t(x << 5)) >> 5;
    this->drawing_y_offset = (int16_t(y << 5)) >> 5;
}

void GPU::gp0_mask_bit_setting() {
    uint32_t p_val = (*this->gp0_command)[0];
    this->force_set_mask_bit = (p_val & 1) != 0;
    this->preserve_masked_pixels = (p_val & 2) != 0;
}

void GPU::gp0_clear_cache() { printf("GP0: Clear cache\n"); }

void GPU::gp0_nop() { return; }

void GPU::gp0_quad_mono_opaque() { printf("GP0: Draw quad\n"); }

void GPU::gp0_draw_mode() {
    uint32_t p_val = (*this->gp0_command)[0];
    this->page_base_x = uint8_t(p_val & 0xf);
    this->page_base_y = uint8_t((p_val >> 4) & 1);
    this->semi_transparency = uint8_t((p_val >> 5) & 3);

    switch ((p_val >> 7) & 3) {
    case 0:
        this->texture_depth = TextureDepth::T4Bit;
        break;
    case 1:
        this->texture_depth = TextureDepth::T8Bit;
        break;
    case 2:
        this->texture_depth = TextureDepth::T15Bit;
        break;
    default:
        printf("Unhandled textured depth: %d\n",
               ((p_val >> 7) & 3));
        std::terminate();
    }

    this->dithering = ((p_val >> 9) & 1) != 0;
    this->draw_to_display = ((p_val >> 10) & 1) != 0;
    this->texture_disable = ((p_val >> 11) & 1) != 0;
    this->rectangle_texture_x_flip = ((p_val >> 12) & 1) != 0;
    this->rectangle_texture_y_flip = ((p_val >> 13) & 1) != 0;
}

void GPU::gp1_reset(uint32_t p_val) {
    this->interrupt = false;
    this->page_base_x = 0;
    this->page_base_y = 0;
    this->semi_transparency = 0;
    this->texture_depth = TextureDepth::T4Bit;
    this->texture_window_x_mask = 0;
    this->texture_window_y_mask = 0;
    this->texture_window_x_offset = 0;
    this->texture_window_y_offset = 0;
    this->dithering = false;
    this->draw_to_display = false;
    this->texture_disable = false;
    this->rectangle_texture_x_flip = false;
    this->rectangle_texture_y_flip = false;
    this->drawing_area_left = 0;
    this->drawing_area_top = 0;
    this->drawing_area_right = 0;
    this->drawing_area_bottom = 0;
    this->drawing_x_offset = 0;
    this->drawing_y_offset = 0;
    this->force_set_mask_bit = false;
    this->preserve_masked_pixels = false;
    this->dma_direction = DmaDirection::Off;
    this->display_disabled = true;
    this->hres = HorizontalRes::from_fields(0, 0);
    this->vres = VerticalRes::Y240Lines;
    this->vmode = VMode::Ntsc;
    this->interlaced = true;
    this->display_depth = DisplayDepth::D15Bits;
    this->display_vram_x_start = 0;
    this->display_vram_y_start = 0;
    this->display_horiz_start = 0x200;
    this->display_horiz_end = 0xc00;
    this->display_line_start = 0x10;
    this->display_line_end = 0x100;
}

void GPU::gp1_display_mode(uint32_t p_val) {
    uint8_t hr1 = uint8_t(p_val & 3);
    uint8_t hr2 = uint8_t((p_val >> 6) & 1);

    this->hres = HorizontalRes::from_fields(hr1, hr2);

    if ((p_val & 0x4) != 0)
        this->vres = VerticalRes::Y240Lines;
    else
        this->vres = VerticalRes::Y480Lines;

    if ((p_val & 0x8) != 0)
        this->vmode = VMode::Ntsc;
    else
        this->vmode = VMode::Pal;

    if ((p_val & 0x10) != 0)
        this->display_depth = DisplayDepth::D24Bits;
    else
        this->display_depth = DisplayDepth::D15Bits;

    this->interlaced = (p_val & 0x20) != 0;

    if ((p_val & 0x80) != 0) {
        printf("Unsupported_display_mode: 0x%x\n", p_val);
        std::terminate();
    }
}

void GPU::gp1_dma_direction(uint32_t p_val) {
    static constexpr DmaDirection directions[] = {
        DmaDirection::Off, DmaDirection::Fifo,
        DmaDirection::CpuToGp0, DmaDirection::VRamToCpu};

    const uint32_t index = p_val & 3;

    if (index < sizeof(directions)) {
        this->dma_direction = directions[index];
    } else {
        printf("ERROR: Wrong gp_1_dma_direction: %d\n", index);
        std::terminate();
    }
}

void GPU::gp1_display_vram_start(uint32_t p_val) {
    this->display_vram_x_start = uint16_t(p_val & 0x3fe);
    this->display_vram_y_start = uint16_t((p_val >> 10) & 0x3fe);
}

void GPU::gp1_display_horizontal_range(uint32_t p_val) {
    this->display_horiz_start = uint16_t(p_val & 0xfff);
    this->display_horiz_end = uint16_t((p_val >> 12) & 0xfff);
}

void GPU::gp1_display_vertical_range(uint32_t p_val) {
    this->display_line_start = uint16_t(p_val & 0x3ff);
    this->display_line_end = uint16_t((p_val >> 10) & 0x3ff);
}

uint32_t GPU::read() { return 0; }
