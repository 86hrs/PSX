#pragma once
#include <cstdint>

struct RAM {
  // RAM BUFFER
  uint8_t data[2*1024*1024];

  RAM();
  ~RAM() = default;

  uint32_t load32(uint32_t offset);
  void store32(uint32_t offset, uint32_t value);

  void store8(uint32_t offset, uint8_t value);
  uint8_t load8(uint32_t offset);
};


