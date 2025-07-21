#pragma once
#include <vector>

struct RAM {
  // RAM BUFFER
  std::vector<uint8_t> data;

  RAM();
  ~RAM() = default;

  uint32_t load32(uint32_t offset);
  void store32(uint32_t offset, uint32_t value);
};


