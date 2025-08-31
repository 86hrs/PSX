// gl_buffer.cc
#include "gl_buffer.h"
#include "structs.h"
#include <cstring>
#include <algorithm>

template <typename T>
Buffer<T>::Buffer()
    : dirty(false), dirty_start(VERTEX_BUFFER_LEN),
      dirty_end(0) {
    glGenBuffers(1, &object);
    glBindBuffer(GL_ARRAY_BUFFER, object);

    // Allocate buffer with initial data
    shadow_copy.resize(VERTEX_BUFFER_LEN, T());

    glBufferData(GL_ARRAY_BUFFER, sizeof(T) * VERTEX_BUFFER_LEN,
                 shadow_copy.data(), GL_DYNAMIC_DRAW);
}

template <typename T> Buffer<T>::~Buffer() {
    glDeleteBuffers(1, &object);
}

template <typename T>
void Buffer<T>::set(uint32_t index, const T &value) {
    if (index >= VERTEX_BUFFER_LEN) {
        throw std::out_of_range("Buffer index out of range");
    }

    // Update shadow copy
    shadow_copy[index] = value;

    // Update dirty range
    dirty_start = std::min(dirty_start, index);
    dirty_end = std::max(dirty_end, index + 1);
    dirty = true;
}

template <typename T> void Buffer<T>::flush() {
    if (!dirty)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, object);

    // Calculate byte range to update
    GLintptr offset = dirty_start * sizeof(T);
    GLsizeiptr size = (dirty_end - dirty_start) * sizeof(T);

    // Update only the dirty region
    glBufferSubData(GL_ARRAY_BUFFER, offset, size,
                    shadow_copy.data() + dirty_start);

    // Reset dirty state
    dirty = false;
    dirty_start = VERTEX_BUFFER_LEN;
    dirty_end = 0;
}

template class Buffer<Color>;
template class Buffer<Position>;
