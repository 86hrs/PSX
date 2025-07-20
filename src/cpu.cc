
#include "cpu.h"
#include "instruction.h"
#include "interconnect.h"
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>

CPU::CPU(Interconnect *p_inter) {
    this->program_counter = 0xbfc00000;
    this->inter = p_inter;
    this->status_register = 0;

    for (uint32_t &reg : this->regs) {
        reg = 0;
    }

    this->set_reg(0, 0);
}

void CPU::run_next_instruction() {
    Instruction instruction = this->next_instruction;

    uint32_t instruction_fetch = this->load32(this->program_counter);
    Instruction next_instruction(instruction_fetch);
    this->next_instruction = next_instruction;

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
    if ((this->status_register & 0x10000) != 0) {
        std::cout << "LOG: Ignoring store while cache is isolated\n";
        return;
    }
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

void CPU::op_jmp(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_jump();

    this->program_counter = (this->program_counter & 0xf0000000) | (i << 2);
}

void CPU::op_or(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) | this->get_reg(t);

    this->set_reg(d, v);
}

void CPU::op_cop0(Instruction p_instruction) {
    switch (p_instruction.cop_opcode()) {
    case 0b00100: {
        this->op_mtc0(p_instruction);
        break;
    }
    default:
        std::cout << "Error: Unhandled cop0 instruction" << std::hex
                  << p_instruction.cop_opcode() << "\n";
        std::terminate();
    }
    return;
}

void CPU::op_mtc0(Instruction p_instruction) {
    uint32_t cpu_r = p_instruction.t();
    uint32_t cop_r = p_instruction.d();

    uint32_t v = this->get_reg(cpu_r);

    switch (cop_r) {
    case 12:
        this->status_register = v;
        break;
    default:
        std::cout << "Error: Unhandled cop0 register";
        std::terminate();
    }
}
void CPU::op_bne(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    if (this->get_reg(s) != this->get_reg(t)) {
        this->branch(i);
    }
}

void CPU::op_addi(Instruction instruction) {
    int32_t i = static_cast<int32_t>(instruction.imm_se());
    uint32_t t = instruction.t();
    uint32_t s = instruction.s();
    int32_t s_val = static_cast<int32_t>(this->get_reg(s));
    
    int32_t result;
    if (__builtin_add_overflow(s_val, i, &result)) {
        throw std::overflow_error("ADDI overflow");
    }
   this->set_reg(t, static_cast<uint32_t>(result));
}

void CPU::branch(uint32_t p_offset) {
    uint32_t offset = p_offset << 2;

    uint32_t pc = this->program_counter;

    pc += offset;
    pc -= 4;

    this->program_counter = pc;
}

void CPU::decode_and_execute_instruction(Instruction p_instruction) {
    switch (p_instruction.function()) {
    case 0b000000: {
        switch (p_instruction.subfunction()) {
        case 0b000000: {
            this->op_sll(p_instruction);
            break;
        }
        case 0b100101: {
            this->op_or(p_instruction);
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
    case 0b000010: {
        this->op_jmp(p_instruction);
        break;
    }
    case 0b010000: {
        this->op_cop0(p_instruction);
        break;
    }
    case 0b000101: {
        this->op_bne(p_instruction);
        break;
    }
    case 0b001000: {
        this->op_addi(p_instruction);
        break;
    }
    default:
        std::cout << "Unhandled instruction << " << std::hex
                  << p_instruction.opcode << "\n";
        std::terminate();
    }
}

void CPU::run() {
    while (true) {
        this->run_next_instruction();
    }
}
