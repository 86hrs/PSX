#include "bios.h"
#include <cstdint>
#include <fstream>
#include <iostream>

template uint8_t Bios::load<uint8_t>(uint32_t);
template uint32_t Bios::load<uint32_t>(uint32_t);

Bios::Bios(const char *filename) {
    std::ifstream ifs(filename,
                      std::ios::binary | std::ios::ate);

    if (!ifs)
        throw std::runtime_error(
            std::string("Failed to open BIOS file: ") +
            filename);

    constexpr size_t EXPECTED_SIZE = 512 * 1024; // 512KB
    auto file_size = ifs.tellg();

    if (file_size != EXPECTED_SIZE) {
        throw std::runtime_error(
            "Invalid BIOS file size. Expected " +
            std::to_string(EXPECTED_SIZE) + " bytes, got " +
            std::to_string(file_size));
    }

    this->data.resize(EXPECTED_SIZE);
    ifs.seekg(0);

    if (!ifs.read(reinterpret_cast<char *>(this->data.data()),
                  EXPECTED_SIZE)) {
        throw std::runtime_error("Failed to read BIOS data");
    }
}

template <class T> T Bios::load(uint32_t p_offset) {
    switch (sizeof(T)) {
    case 1:
        return this->data[p_offset];
    case 4:
        uint32_t b0 = this->data[p_offset + 0];
        uint32_t b1 = this->data[p_offset + 1];
        uint32_t b2 = this->data[p_offset + 2];
        uint32_t b3 = this->data[p_offset + 3];

        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }
}

