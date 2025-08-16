#include "commandbuffer.h"
#include <cstdio>
#include <cstring>
#include <exception>

CommmandBuffer::CommmandBuffer() {
    memset(this->buffer, 0, sizeof(this->buffer));
    this->length = 0;
}

void CommmandBuffer::clear() { this->length = 0; }

void CommmandBuffer::push_word(uint32_t p_word) {
    this->buffer[this->length] = p_word;
    this->length += 1;
}
uint32_t CommmandBuffer::operator[](uint32_t p_index) const {
    if (p_index >= this->length) {
        printf("Index: %d out of range\n", p_index);
        std::terminate();
    }
    return this->buffer[p_index];
}
