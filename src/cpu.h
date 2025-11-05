#pragma once
#include <cstddef>
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
    uint32_t next_program_counter;
    uint32_t current_program_counter;
    uint32_t regs[32];
    uint32_t status_register;
    uint32_t cause_register;
    uint32_t epc_register;
    // Hi register for division remainder and multiplication high
    // result
    uint32_t hi;
    // Lo register for division quotient and multiplication low
    // result
    uint32_t lo;

    uint64_t opcode_count;

    bool branch_occured = false;
    bool delay_slot = false;

    uint32_t out_regs[32];
    uint32_t load_reg, load_val;

    Interconnect *inter;

    CPU(Interconnect *);
    ~CPU() = default;

    enum Exception : uint32_t {
        SysCall = 0x8,
        ArithmeticOverflow = 0xc,
        /// Address error on load
        LoadAddressError = 0x4,
        /// Address error on store
        StoreAddressError = 0x5,
        IllegalInstruction = 0xa,
        // Breakpoint (caused by the BREAK opcode)
        Break = 0x9,
        // Unsupported cocpu operation
        CoprocessorError = 0xb,
    };

    void op_cop1(Instruction);
    void op_cop2(Instruction);
    void op_cop3(Instruction);

    void run();
    void print();
    void run_next_instruction();
    void execute_instruction(Instruction);

    void exception(Exception);

    void branch(uint32_t p_offset);

    uint32_t load32(uint32_t addr);
    uint16_t load16(uint32_t addr);
    uint8_t load8(uint32_t addr);

    void store32(uint32_t addr, uint32_t val);
    void store16(uint32_t addr, uint16_t val);
    void store8(uint32_t addr, uint8_t val);

    uint32_t get_reg(uint32_t idx);
    void set_reg(uint32_t idx, uint32_t val);

    typedef void (CPU::*op_handler)(Instruction);

    op_handler rtype_dispatch[63] = {nullptr };
    op_handler main_dispatch[63] = { nullptr };

    void op_lui(Instruction);
    void op_ori(Instruction);
    void op_sw(Instruction);
    void op_lw(Instruction);
    void op_sll(Instruction);
    void op_addiu(Instruction);
    void op_jmp(Instruction);
    void op_or(Instruction);
    void op_cop0(Instruction);
    void op_mtc0(Instruction);
    void op_mfc0(Instruction);
    void op_bne(Instruction);
    void op_addi(Instruction);
    void op_sh(Instruction);
    void op_stlu(Instruction);
    void op_slti(Instruction);
    void op_addu(Instruction);
    void op_jal(Instruction);
    void op_andi(Instruction);
    void op_sb(Instruction);
    void op_jr(Instruction);
    void op_lb(Instruction);
    void op_beq(Instruction);
    void op_and(Instruction);
    void op_syscall(Instruction);
    void op_jalr(Instruction);
    void op_add(Instruction);
    void op_bgtz(Instruction);
    void op_blez(Instruction);
    void op_lbu(Instruction);
    void op_bxx(Instruction);
    void op_subu(Instruction);
    void op_sub(Instruction);
    void op_sra(Instruction);
    void op_div(Instruction);
    void op_divu(Instruction);
    void op_sltiu(Instruction);
    void op_mflo(Instruction);
    void op_srl(Instruction);
    void op_mfhi(Instruction);
    void op_slt(Instruction);
    void op_mtlo(Instruction);
    void op_mthi(Instruction);
    void op_rfe(Instruction);
    void op_lhu(Instruction);
    void op_sllv(Instruction);
    void op_lh(Instruction);
    void op_nor(Instruction);
    void op_srav(Instruction);
    void op_xor(Instruction);
    void op_srlv(Instruction);
    void op_multu(Instruction);
    void op_break(Instruction);
    void op_mult(Instruction);
    void op_xori(Instruction);
    void op_lwl(Instruction);
    void op_lwr(Instruction);
    void op_swl(Instruction);
    void op_swr(Instruction);

    void op_lwc0(Instruction);
    void op_lwc1(Instruction);
    void op_lwc2(Instruction);
    void op_lwc3(Instruction);

    void op_swc0(Instruction);
    void op_swc1(Instruction);
    void op_swc2(Instruction);
    void op_swc3(Instruction);
    
    

    void op_illegal(Instruction);
};
