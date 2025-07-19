#pragma once
#include "bios.h"

struct Interconnect {
  Bios* bios;

  Interconnect(Bios*);
  ~Interconnect() = default;

  uint32_t load32(uint32_t);
  void store32(uint32_t, uint32_t);
};
