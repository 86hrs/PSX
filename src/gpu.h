#pragma once
#include <cstdint>

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

  // Create a new HorizontalRes instance from the 2-bit field hr1
  // and the one-bit field hr2
  static HorizontalRes from_fields(uint8_t hr1, uint8_t hr2) {
    uint8_t hr = (hr2 & 1) | ((hr1 & 3) << 1);
    return HorizontalRes{hr};
  }
  // Retrieve value of bits [18:16] of the status register
  uint32_t into_status() const { return static_cast<uint32_t>(value) << 16; }
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

  GPU();
  ~GPU() = default;

  uint32_t status();
  void gp0(uint32_t p_val);
  void gp1(uint32_t p_val);

  void gp0_draw_mode(uint32_t p_val);
  void gp0_drawing_area_top_left(uint32_t p_val);
  void gp0_drawing_area_bottom_right(uint32_t p_val);
  void gp0_drawing_offset(uint32_t p_val);
  void gp0_texture_window(uint32_t p_val);

  void gp1_reset(uint32_t p_val);
  void gp1_display_mode(uint32_t p_val);
  void gp1_dma_direction(uint32_t p_val);

  uint32_t read();
};
