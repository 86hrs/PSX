#pragma once
#include <cstdint>
#include <vector>

struct Bios {
  static constexpr uint64_t BIOS_SIZE = 512 * 1024;
  std::vector<uint8_t> data;

  Bios(const char *);
  ~Bios() = default;

  uint32_t load32(uint32_t);
};
