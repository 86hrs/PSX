#include "gl_buffer.h"
#include "structs.h"
#include <stdexcept>
#include <vector>

template <typename T> Buffer<T>::Buffer() {
    this->object = 0;
    this->map = nullptr;
    
    glGenBuffers(1, &object);
    glBindBuffer(GL_ARRAY_BUFFER, object);
    
    GLsizeiptr size = sizeof(T) * VERTEX_BUFFER_LEN;
    
    // Use traditional buffer data allocation
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    
    // Initialize with default values using a temporary vector
    std::vector<T> initialData(VERTEX_BUFFER_LEN, T());
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, initialData.data());
}

template <typename T> Buffer<T>::~Buffer() {
    glDeleteBuffers(1, &object);
}

template <typename T>
void Buffer<T>::set(uint32_t index, const T &value) {
    if (index < 0 || index >= VERTEX_BUFFER_LEN) {
        throw std::out_of_range("Buffer index out of range");
    }
    
    // Update immediately or mark as dirty for batch updates
    glBindBuffer(GL_ARRAY_BUFFER, object);
    glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(T), sizeof(T), &value);
}

template class Buffer<Color>;
template class Buffer<Position>;
