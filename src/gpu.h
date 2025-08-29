#pragma once
#include "commandbuffer.h"
#include <cstdint>
#include "renderer.h"

/// Depth of the pixel values in a texture page
enum TextureDepth : uint32_t {
  /// 4 bits per pixel
  T4Bit = 0,
  /// 8 bits per pixel
  T8Bit = 1,
  /// 15 bits per pixel
  T15Bit = 2,
};
/// Interlaced output splits each frame in two fields
enum Field : uint32_t {
  /// Top field (odd lines).
  Top = 1,
  /// Bottom field (even lines)
  Bottom = 0,
};

/// Video output vertical resolution
enum VerticalRes : uint32_t {
  /// 240 lines
  Y240Lines = 0,
  /// 480 lines (only available for interlaced output)
  Y480Lines = 1,
};

// Video modes
enum VMode : uint32_t {
  /// NTSC: 480i60H
  Ntsc = 0,
  /// PAL: 576i50Hz
  Pal = 1,
};

// Display area color depth
enum DisplayDepth : uint32_t {
  /// 15 bits per pixel
  D15Bits = 0,
  /// 24 bits per pixel
  D24Bits = 1,
};

enum DmaDirection : uint32_t {
  Off = 0,
  Fifo = 1,
  CpuToGp0 = 2,
  VRamToCpu = 3,
};

struct HorizontalRes {
  uint8_t value;

  // Create a new HorizontalRes instance from the 2-bit field
  // hr1 and the one-bit field hr2
  static HorizontalRes from_fields(uint8_t hr1, uint8_t hr2) {
    uint8_t hr = (hr2 & 1) | ((hr1 & 3) << 1);
    return HorizontalRes{hr};
  }
  // Retrieve value of bits [18:16] of the status register
  uint32_t into_status() const { return static_cast<uint32_t>(value) << 16; }
};

enum Gp0Mode {
  // Default mode: handling commands
  Command,
  // Loading an image into VRAM
  ImageLoad,
};

struct Poisition {
  GLshort x;
  GLshort y;
};

struct GPU {
  uint8_t page_base_x;
  uint8_t page_base_y;

  uint8_t semi_transparency;
  TextureDepth texture_depth;

  bool dithering;
  bool draw_to_display;
  bool force_set_mask_bit;
  bool texture_disable;
  bool preserve_masked_pixels;

  bool rectangle_texture_x_flip;
  bool rectangle_texture_y_flip;

  Field field;

  VerticalRes vres;
  HorizontalRes hres;

  VMode vmode;

  Gp0Mode gp0_mode;

  DisplayDepth display_depth;

  bool interlaced;
  bool display_disabled;

  bool interrupt;
  DmaDirection dma_direction;

  uint8_t texture_window_x_mask;
  uint8_t texture_window_y_mask;
  uint8_t texture_window_x_offset;
  uint8_t texture_window_y_offset;

  uint16_t drawing_area_left;
  uint16_t drawing_area_top;
  uint16_t drawing_area_right;
  uint16_t drawing_area_bottom;

  int16_t drawing_x_offset;
  int16_t drawing_y_offset;

  uint16_t display_vram_x_start;
  uint16_t display_vram_y_start;

  uint16_t display_horiz_start;
  uint16_t display_horiz_end;

  uint16_t display_line_start;
  uint16_t display_line_end;

  Renderer renderer;

  GPU(CommmandBuffer *);
  ~GPU() = default;

  // Buffer containing the current GP0 command
  CommmandBuffer *gp0_command;
  // Remaining words for the current GP0 command
  uint32_t gp0_command_remaining;
  // Pointer to the method implementing the current GPX commnad
  void (GPU::*gp0_command_ptr)(void);

  uint32_t status();
  void gp0(uint32_t p_val);
  void gp1(uint32_t p_val);

  void gp0_draw_mode();
  void gp0_drawing_area_top_left();
  void gp0_drawing_area_bottom_right();
  void gp0_drawing_offset();
  void gp0_texture_window();
  void gp0_mask_bit_setting();
  void gp0_nop();
  void gp0_clear_cache();
  void gp0_image_load();
  void gp0_image_store();

  void gp0_quad_mono_opaque();
  void gp0_quad_shaded_opaque();
  void gp0_triangle_shaded_opaque();
  void gp0_quad_texture_blend_opaque();

  void gp1_reset(uint32_t p_val);
  void gp1_acknowledge_irq();
  void gp1_display_enable(uint32_t p_val);
  void gp1_display_mode(uint32_t p_val);
  void gp1_dma_direction(uint32_t p_val);
  void gp1_display_vram_start(uint32_t p_val);
  void gp1_display_horizontal_range(uint32_t p_val);
  void gp1_display_vertical_range(uint32_t p_val);
  void gp1_reset_command_buffer();

  uint32_t read();
};
