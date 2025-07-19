#pragma once
#include <cstdint>
#include "interconnect.h"
#include "instruction.h"

/*
KUSEG      KSEG0     KSEG1       Length    Description
0x00000000 0x80000000 0xa0000000 2048K     Main RAM
0x1f000000 0x9f000000 0xbf000000 8192K     Expansion Region 1
0x1f800000 0x9f800000 0xbf800000 1K        Scratchpad
0x1f801000 0x9f801000 0xbf801000 8K        Hardware registers
0x1fc00000 0x9fc00000 0xbfc00000 512K      BIOS ROM
*/

/*
KSEG2      LENGTH   Description
0xfffe0000 512B     I/O Ports
*/

struct CPU {
    uint32_t program_counter;
    uint32_t regs[32];

    Interconnect* inter;

    CPU(Interconnect*);
    ~CPU() = default;

    void run();
    void increment_program_counter();
    void run_next_instruction();
    void decode_and_execute_instruction(Instruction);
    uint32_t load32(uint32_t);
    void store32(uint32_t, uint32_t);
    uint32_t get_reg(uint32_t);
    void set_reg(uint32_t, uint32_t);

    void op_lui(Instruction);
    void op_ori(Instruction);
    void op_sw(Instruction);
    void op_sll(Instruction);
    void op_addiu(Instruction);
};
