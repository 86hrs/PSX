
#include "cpu.h"
#include "instruction.h"
#include "interconnect.h"
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>

CPU::CPU(Interconnect *p_inter) {
    this->program_counter = 0xbfc00000;
    this->next_program_counter = this->program_counter + 4;
    this->inter = p_inter;
    this->status_register = 0;
    this->opcode_count = 0;

    for (uint32_t &reg : this->regs) {
        reg = 0;
    }

    for (uint32_t &out_reg : this->out_regs) {
        out_reg = 0;
    }

    this->load_reg = 0;
    this->load_val = 0;

    // $0 always zero
    this->set_reg(0, 0);
}

void CPU::run_next_instruction() {
    this->program_counter = this->next_program_counter;
    if (this->program_counter % 4 != 0) {
        this->exception(Exception::LoadAddressError);
        return;
    }

    Instruction instruction =
        Instruction(this->load32(this->program_counter));
    this->current_program_counter = this->program_counter;

    this->set_reg(load_reg, load_val);
    this->load_reg = 0;
    this->load_val = 0;

    this->decode_and_execute_instruction(instruction);

    this->next_program_counter = this->program_counter + 4;

    this->opcode_count += 1;

    std::copy(std::begin(this->out_regs),
              std::end(this->out_regs), std::begin(this->regs));
}

uint32_t CPU::load32(uint32_t p_addr) {
    return this->inter->load32(p_addr);
}
uint8_t CPU::load8(uint32_t p_addr) {
    return this->inter->load8(p_addr);
}

void CPU::store32(uint32_t addr, uint32_t val) {
    this->inter->store32(addr, val);
}
void CPU::store16(uint32_t addr, uint16_t val) {
    this->inter->store16(addr, val);
}
void CPU::store8(uint32_t addr, uint8_t val) {
    this->inter->store8(addr, val);
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
        std::cout << program_counter << std::endl;
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
        std::cout << "SH LOG: Ignoring store while cache is "
                     "isolated\n";
        return;
    }
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;
    uint32_t v = this->get_reg(t);

    this->store16(addr, (uint16_t)v);
}
// Store byte
void CPU::op_sb(Instruction p_instruction) {
    if ((this->status_register & 0x10000) != 0) {
        std::cout << "SB LOG: Ignoring store while cache is "
                     "isolated\n";
        return;
    }

    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;
    uint32_t v = this->get_reg(t);

    this->store8(addr, (uint8_t)v);
}
// Load Word
void CPU::op_lw(Instruction p_instruction) {
    if ((this->status_register & 0x10000) != 0) {
        std::cout
            << "LW LOG: Ignoring load while cache is isolated\n";
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
// Load byte
void CPU::op_lb(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    int8_t v = this->load32(addr);

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
    this->next_program_counter =
        (this->current_program_counter & 0xf0000000) | (i << 2);
}

void CPU::op_jal(Instruction p_instruction) {
    uint32_t return_address = this->next_program_counter;
    this->set_reg(31, return_address);
    this->op_jmp(p_instruction);
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
    case 0b00000: {
        this->op_mfc0(p_instruction);
        break;
    }
    case 0b00100: {
        this->op_mtc0(p_instruction);
        break;
    }
    default:
        std::cout << "CPU::OP_COP: Unhandled cop0 instruction"
                  << std::hex << p_instruction.cop_opcode()
                  << "\n";
        std::terminate();
    }
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
            throw std::runtime_error(
                "Unhandled write to CAUSE register.");
        }
        break;
    default:
        throw std::runtime_error("Unhandled cop0 register " +
                                 std::to_string(cop_r));
        break;
    }
}

void CPU::op_mfc0(Instruction p_instruction) {
    uint32_t cpu_r = p_instruction.t();
    uint32_t cop_r = p_instruction.d();

    uint32_t v = 0;

    switch (cop_r) {
    case 12:
        v = this->status_register;
        break;
    case 13:
        // Cause register
        v = this->cause_register;
        break;
    case 14:
        v = this->epc_register;
        break;
    default:
        std::cout << "Unhandled read from cop0r: " << cop_r
                  << "\n";
        break;
    }
    this->load_reg = cpu_r;
    this->load_val = v;
}
void CPU::op_bne(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    if (this->get_reg(s) != this->get_reg(t)) {
        this->branch(i);
    }
}
void CPU::op_beq(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    if (this->get_reg(s) == this->get_reg(t)) {
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
        this->exception(Exception::ArithmeticOverflow);
        return;
    }
    this->set_reg(t, static_cast<uint32_t>(result));
}

void CPU::op_stlu(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) < this->get_reg(t);
    this->set_reg(d, v);
}

void CPU::op_addu(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();
    uint32_t d = p_instruction.d();

    uint32_t v = this->get_reg(s) + this->get_reg(t);

    this->set_reg(d, v);
}
void CPU::op_andi(Instruction p_instruction) {
    uint32_t i = p_instruction.imm();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t v = this->get_reg(s) & i;
    this->set_reg(t, v);
}
void CPU::op_jr(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    this->program_counter = this->get_reg(s);
}
void CPU::op_and(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) & this->get_reg(t);

    this->set_reg(d, v);
}

void CPU::branch(uint32_t offset) {
    this->next_program_counter += (offset << 2);
}

void CPU::exception(Exception cause) {
    uint32_t handler = 0x0;

    if ((this->status_register & (1 << 22)) != 0)
        handler = 0xbfc00180;
    else
        handler = 0x80000080;

    uint32_t mode = this->status_register & 0x3f;

    this->status_register &= ~0x3f;
    this->status_register |= (mode << 2) & 0x3f;

    this->cause_register = (cause) << 2;
    this->epc_register = this->current_program_counter;

    this->program_counter = handler;
    this->next_program_counter += 4;
}

void CPU::op_syscall(Instruction p_instruction) {
    (void)p_instruction;
    this->exception(Exception::SysCall);
}

void CPU::decode_and_execute_instruction(
    Instruction p_instruction) {
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
        case 0b100001: {
            this->op_addu(p_instruction);
            break;
        }
        case 0b001000: {
            this->op_jr(p_instruction);
            break;
        }
        case 0b100100: {
            this->op_and(p_instruction);
            break;
        }
        default:
            std::cout << "CPU::DECODE: Unhandled special "
                         "instruction: 0x"
                      << std::hex << p_instruction.opcode
                      << std::endl;
            std::cout << "Opcodes computed: " << std::dec
                      << this->opcode_count << "\n";
            std::cout << "PC: " << std::hex
                      << this->program_counter << "\n";
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
    case 0b100011: {
        this->op_lw(p_instruction);
        break;
    }
    case 0b101001: {
        this->op_sh(p_instruction);
        break;
    }
    case 0b000011: {
        this->op_jal(p_instruction);
        break;
    }
    case 0b001100: {
        this->op_andi(p_instruction);
        break;
    }
    case 0b101000: {
        this->op_sb(p_instruction);
        break;
    }
    case 0b100000: {
        this->op_lb(p_instruction);
        break;
    }
    case 0b000100: {
        this->op_beq(p_instruction);
        break;
    }
    default:
        std::cout << "CPU::DECODE: Unhandled instruction: 0x"
                  << std::hex << p_instruction.opcode << "\n";
        std::cout << "Opcodes computed: " << std::dec
                  << this->opcode_count << std::endl;
        std::terminate();
    }
}

void CPU::run() {
    while (1) {
        this->run_next_instruction();
    }
}
