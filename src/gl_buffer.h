// gl_buffer.h
#pragma once
#include "glad.h"
#include <cstddef>
#include <vector>

template <typename T> struct Buffer {
    GLuint object;
    std::vector<T> shadow_copy;
    bool dirty;
    uint32_t dirty_start;
    uint32_t dirty_end;

    static const uint32_t VERTEX_BUFFER_LEN = 64 * 1024;

    Buffer();
    ~Buffer();

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    void set(uint32_t index, const T &value);
    void flush(); // Flush changes to GPU
    GLuint id() const { return object; };
    void bind() const { glBindBuffer(GL_ARRAY_BUFFER, object); };
};
