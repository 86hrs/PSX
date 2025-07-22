
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
    this->next_instruction = Instruction(0);

    for (uint32_t &reg : this->regs) {
        reg = 0;
    }

    for (uint32_t &out_reg : this->out_regs) {
        out_reg = 0;
    }

    this->load_reg = 0;
    this->load_val = 0;

    this->set_reg(0, 0);
}

void CPU::run_next_instruction() {
    Instruction instruction = this->next_instruction;
    this->next_instruction = Instruction(this->load32(this->program_counter));

    this->set_reg(load_reg, load_val);
    this->load_reg = 0;
    this->load_val = 0;

    this->decode_and_execute_instruction(instruction);

    std::copy(std::begin(this->out_regs), std::end(this->out_regs),
              std::begin(this->regs));
}

uint32_t CPU::load32(uint32_t p_addr) { return this->inter->load32(p_addr); }

void CPU::store32(uint32_t addr, uint32_t val) {
    this->inter->store32(addr, val);
}
void CPU::store16(uint32_t addr, uint16_t val) {
    this->inter->store16(addr, val);
}

uint32_t CPU::get_reg(uint32_t idx) { return this->regs[idx]; }

void CPU::set_reg(uint32_t idx, uint32_t val) {
    this->out_regs[idx] = val;
    // $zero always zero
    this->out_regs[0] = 0;
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
// Store half word
void CPU::op_sh(Instruction p_instruction) {
    if ((this->status_register & 0x10000) != 0) {
        std::cout << "LOG: Ignoring store while cache is isolated\n";
        return;
    }
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;
    uint32_t v = this->get_reg(t);

    this->store16(addr, (uint16_t)v);
}
// Load Word
void CPU::op_lw(Instruction p_instruction) {
    if ((this->status_register & 0x10000) != 0) {
        std::cout << "LOG: Ignoring load while cache is isolated\n";
        return;
    }
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    uint32_t v = this->load32(addr);

    this->load_reg = t;
    this->load_val = v;
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
    case 3:
    case 5:
    case 6:
    case 7:
    case 9:
    case 11:
        // Breakpoints registers
        if (v != 0) {
            throw std::runtime_error("Unhandled write to cop0r" +
                                     std::to_string(cop_r));
        }
        break;
    case 12:
        this->status_register = v;
        break;

    case 13:
        // Cause register
        if (v != 0) {
            throw std::runtime_error("Unhandled write to CAUSE register.");
        }
        break;
    default:
        throw std::runtime_error("Unhandled cop0 register " +
                                 std::to_string(cop_r));
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
    uint32_t i = instruction.imm_se();
    uint32_t t = instruction.t();
    uint32_t s = instruction.s();
    int32_t s_val = static_cast<int32_t>(this->get_reg(s));

    int32_t result;
    if (__builtin_add_overflow(s_val, i, &result)) {
        throw std::overflow_error("ADDI overflow");
    }
    this->set_reg(t, static_cast<uint32_t>(result));
}

void CPU::op_stlu(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v =this->get_reg(s) < this->get_reg(t);
    this->set_reg(d, v);    
}

void CPU::branch(uint32_t offset) { this->program_counter += (offset << 2) + 4; }

void CPU::decode_and_execute_instruction(Instruction p_instruction) {
    this->advance_program_counter = true;
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
        case 0b101011: {
            this->op_stlu(p_instruction);
            break;
        }
        default:
            std::cout << "Unimplemented: " << std::hex
                      << p_instruction.subfunction() << std::endl;
            std::terminate();
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
        advance_program_counter = false;
        this->op_jmp(p_instruction);
        break;
    }
    case 0b010000: {
        this->op_cop0(p_instruction);
        break;
    }
    case 0b000101: {
        advance_program_counter = false;
        this->op_bne(p_instruction);
        break;
    }
    case 0b001000: {
        this->op_addi(p_instruction);
        break;
    }
    case 0b100011: {
        this->op_lw(p_instruction);
        break;
    }
    case 0b101001: {
        this->op_sh(p_instruction);
        break;
    }
    default:
        std::cout << "Unhandled instruction << " << std::hex
                  << p_instruction.opcode << "\n";
        std::terminate();
    }
    this->opcode_count += 1;
    if (this->advance_program_counter)
        this->program_counter += 4;
}

void CPU::run() {
    while (true) {
        this->run_next_instruction();
    }
}
