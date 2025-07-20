#pragma once
#include <cstdint>

struct Instruction {
    uint32_t opcode;

    explicit Instruction(uint32_t opcode) : opcode(opcode) {}
    ~Instruction() = default;

    // Return bits [31:26] of the instruction
    uint32_t function() const { return opcode >> 26; }

    // Return register index in bits [20:16]
    uint32_t t() const { return (opcode >> 16) & 0x1f; }

    // Return immediate value in bits [15:0]
    uint32_t imm() const { return opcode & 0xffff; }

    // Return immediate value in bits [16:0] as a sign-extended 32bit
    // value
    uint32_t imm_se() const {
        int16_t v = (int16_t)(opcode & 0xffff); // Signed 16-bit value
        return (uint32_t)(int32_t)v;            // Sign-extend to 32 bits first
    }

    // Return register index in bits [25:21]
    uint32_t s() const { return (opcode >> 21) & 0x1f; }

    uint32_t cop_opcode() const { return (opcode >> 21) & 0x1f; }

    // Return register index in bits [15:11]
    uint32_t d() const { return (opcode >> 11) & 0x1f; }

    // Return bits [5:0] of the instruction
    uint32_t subfunction() const { return opcode & 0x3f; }

    // Shift Immediate values are stored in bits [10:6]
    uint32_t shift() const { return (opcode >> 6) & 0x1f; }

    // Jump target stored in bits [25:0]
    uint32_t imm_jump() const { return opcode & 0x3ffffff; }

    // Get raw opcode
    uint32_t raw() const { return opcode; }
};
