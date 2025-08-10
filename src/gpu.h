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

struct Gpu {};
