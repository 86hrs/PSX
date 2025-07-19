
#include "cpu.h"
#include "instruction.h"
#include "interconnect.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>

CPU::CPU(Interconnect *p_inter) {
    this->program_counter = 0xbfc00000;
    this->inter = p_inter;

    for (int i = 0; i < 32; i++)
        this->regs[i] = 0xdeadbeef;
    regs[0] = 0;
}

void CPU::run_next_instruction() {
    uint32_t instruction_fetch = this->load32(this->program_counter);
    Instruction instruction(instruction_fetch);
    increment_program_counter();
    this->decode_and_execute_instruction(instruction);
}

void CPU::increment_program_counter() { this->program_counter += 4; }

uint32_t CPU::load32(uint32_t p_addr) { return this->inter->load32(p_addr); }

void CPU::store32(uint32_t addr, uint32_t val) {
    this->inter->store32(addr, val);
}

uint32_t CPU::get_reg(uint32_t idx) { return this->regs[idx]; }
void CPU::set_reg(uint32_t idx, uint32_t val) {
    this->regs[idx] = val;
    // $zero always zero
    this->regs[0] = 0;
}

// Load Upper Immediate
void CPU::op_lui(Instruction p_instruction) {
    uint32_t i = p_instruction.imm();
    uint32_t t = p_instruction.t();

    // Low 16 bits are 0
    uint32_t v = i << 16;
    this->set_reg(t, v);
}

// Bitwisre Or Immediate
void CPU::op_ori(Instruction p_instruction) {
    uint32_t i = p_instruction.imm();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t v = this->get_reg(s) | i;
    this->set_reg(t, v);
}

// Store Word
void CPU::op_sw(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;
    uint32_t v = this->get_reg(t);

    this->store32(addr, v);
}

// Shift Left Logical
void CPU::op_sll(Instruction p_instruction) {
    uint32_t i = p_instruction.shift();
    uint32_t t = p_instruction.t();
    uint32_t d = p_instruction.d();

    uint32_t v = this->get_reg(t) << i;

    this->set_reg(d, v);
}

void CPU::op_addiu(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t v = this->get_reg(s) + i;

    this->set_reg(t, v);
}

void CPU::decode_and_execute_instruction(Instruction p_instruction) {
    switch (p_instruction.function()) {
    case 0b000000: {
        switch (p_instruction.subfunction()) {
        case 0b000000: {
            this->op_sll(p_instruction);
            break;
        }
        default:
            break;
        };
        break;
    }
    case 0b001111: {
        this->op_lui(p_instruction);
        break;
    }
    case 0b001101: {
        this->op_ori(p_instruction);
        break;
    }
    case 0b101011: {
        this->op_sw(p_instruction);
        break;
    }
    case 0b001001: {
        this->op_addiu(p_instruction);
        break;
    }
    default: {
        std::cout << "Unhandled instruction << " << std::hex
                  << p_instruction.opcode << "\n";
        exit(0);
        break;
    }
    }
}

void CPU::run() {
    while (true) {
        this->run_next_instruction();
    }
}
