#pragma once
#include "GLFW/glfw3.h"

struct Position {
  GLshort x;
  GLshort y;

  static Position from_gp0(uint32_t p_val) {
    int16_t x = (int16_t)p_val;
    int16_t y = int16_t(p_val >> 16);
    return Position{x, y};
  };
};

struct Color {
  GLubyte r;
  GLubyte g;
  GLubyte b;

  static Color from_gp0(uint32_t p_val) {
    uint8_t r = (uint8_t)p_val;
    uint8_t g = uint8_t(p_val >> 8);
    uint8_t b = uint8_t(p_val >> 16);

    return Color{r, g, b};
  };
};
