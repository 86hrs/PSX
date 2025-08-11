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
struct Gpu {
  uint8_t page_base_x;
  uint8_t page_base_y;

  uint8_t semi_transparency;
  TextureDepth texture_depth;

  bool dithering;
  bool draw_to_display;
  bool force_set_mask_bit;
  bool texture_disable;
  bool preserve_masked_pixels;

  Field field;

  VerticalRes vres;
  HorizontalRes hres;

  VMode vmode;

  DisplayDepth display_depth;

  bool interlaced;
  bool display_disabled;

  bool interrupt;
  DmaDirection dma_direction;  

  Gpu();
  ~Gpu() = default;

  uint32_t status();
};
