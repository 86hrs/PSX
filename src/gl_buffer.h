#pragma once
#include "glad.h"
#include "GLFW/glfw3.h"

template<typename T>
struct Buffer {
  GLuint object;
  T* map;

  static const uint32_t VERTEX_BUFFER_LEN = 64 * 1024;

  Buffer();
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  void set(uint32_t index, const T& value);
  GLuint id() const { return object; };
};
