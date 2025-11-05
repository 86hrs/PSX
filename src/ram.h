#pragma once
#include <cstdint>

struct RAM {
  // RAM BUFFER
  uint8_t data[2*1024*1024];

  RAM();
  ~RAM() = default;

  template <class T>
  T load(uint32_t p_offset);

  template <class T>
  void store(uint32_t offset, T value);
};


