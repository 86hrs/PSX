#include <glad.h>
#include <stdexcept>
#include "gl_buffer.h"

template <typename T> Buffer<T>::Buffer() {
    this->object = 0;
    this->map = nullptr;

    glGenBuffers(1, &object);
    glBindBuffer(GL_ARRAY_BUFFER, object);

    GLsizeiptr size = sizeof(T) * VERTEX_BUFFER_LEN;

    // Allocate buffer memory with persistent mapping
    glBufferStorage(GL_ARRAY_BUFFER, size, nullptr,
                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

    // Map the buffer
    map = (T *)glMapBufferRange(GL_ARRAY_BUFFER, 0, size,
                                GL_MAP_WRITE_BIT |
                                    GL_MAP_PERSISTENT_BIT);

    if (!map) {
        throw std::runtime_error("Failed to map buffer");
    }

    for (int i = 0; i < VERTEX_BUFFER_LEN; i++) {
        map[i] = T();
    }
}

template <typename T> Buffer<T>::~Buffer() {
    if (map) {
        glBindBuffer(GL_ARRAY_BUFFER, object);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    glDeleteBuffers(1, &object);
}

template <typename T>
void Buffer<T>::set(uint32_t index, const T &value) {
    if (index < 0 || index >= VERTEX_BUFFER_LEN) {
        throw std::out_of_range("Buffer index out of range");
    }
    map[index] = value;
}
