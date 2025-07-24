#include "bios.h"
#include <arm/types.h>
#include <cstdint>
#include <fstream>

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

uint32_t Bios::load32(uint32_t p_offset) {
    uint64_t offset = (uint64_t)p_offset;

    uint32_t b0 = this->data[offset + 0];
    uint32_t b1 = this->data[offset + 1];
    uint32_t b2 = this->data[offset + 2];
    uint32_t b3 = this->data[offset + 3];

    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

uint8_t Bios::load8(uint32_t p_offset) {
    return this->data[p_offset];
}
