#include "cpu.h"
#include "instruction.h"
#include "interconnect.h"
#include <cstdint>
#include <cstdlib>
#include "r3000d.h"
#include <cstdio>
#include <stdexcept>

char buf[1024];

CPU::CPU(Interconnect *p_inter) {
    this->program_counter = 0xbfc00000;
    this->next_program_counter = this->program_counter + 4;
    this->inter = p_inter;
    this->status_register = 0;
    this->opcode_count = 0;
    this->branch_occured = this->delay_slot = false;
    this->hi = this->lo = 0xdeadbeef;

    for (uint32_t &reg : this->regs) {
        reg = 0;
    }

    for (uint32_t &out_reg : this->out_regs) {
        out_reg = 0;
    }

    rtype_dispatch[0b000000] = &CPU::op_sll;
    rtype_dispatch[0b100101] = &CPU::op_or;
    rtype_dispatch[0b101011] = &CPU::op_stlu;
    rtype_dispatch[0b100001] = &CPU::op_addu;
    rtype_dispatch[0b001000] = &CPU::op_jr;
    rtype_dispatch[0b001001] = &CPU::op_jalr;
    rtype_dispatch[0b100100] = &CPU::op_and;
    rtype_dispatch[0b001100] = &CPU::op_syscall;
    rtype_dispatch[0b100000] = &CPU::op_add;
    rtype_dispatch[0b100010] = &CPU::op_sub;
    rtype_dispatch[0b100011] = &CPU::op_subu;
    rtype_dispatch[0b000011] = &CPU::op_sra;
    rtype_dispatch[0b011010] = &CPU::op_div;
    rtype_dispatch[0b011011] = &CPU::op_divu;
    rtype_dispatch[0b010010] = &CPU::op_mflo;
    rtype_dispatch[0b000010] = &CPU::op_srl;
    rtype_dispatch[0b010000] = &CPU::op_mfhi;
    rtype_dispatch[0b101010] = &CPU::op_slt;
    rtype_dispatch[0b010011] = &CPU::op_mtlo;
    rtype_dispatch[0b010001] = &CPU::op_mthi;
    rtype_dispatch[0b000100] = &CPU::op_sllv;
    rtype_dispatch[0b100111] = &CPU::op_nor;
    rtype_dispatch[0b000111] = &CPU::op_srav;
    rtype_dispatch[0b100110] = &CPU::op_xor;
    rtype_dispatch[0b000110] = &CPU::op_srlv;
    rtype_dispatch[0b110000] = &CPU::op_mult;
    rtype_dispatch[0b011001] = &CPU::op_multu;
    rtype_dispatch[0b001101] = &CPU::op_break;

    main_dispatch[0b001111] = &CPU::op_lui;
    main_dispatch[0b001101] = &CPU::op_ori;
    main_dispatch[0b101011] = &CPU::op_sw;
    main_dispatch[0b001001] = &CPU::op_addiu;
    main_dispatch[0b000010] = &CPU::op_jmp;
    main_dispatch[0b010000] = &CPU::op_cop0;
    main_dispatch[0b000101] = &CPU::op_bne;
    main_dispatch[0b001000] = &CPU::op_addi;
    main_dispatch[0b100011] = &CPU::op_lw;
    main_dispatch[0b101001] = &CPU::op_sh;
    main_dispatch[0b000011] = &CPU::op_jal;
    main_dispatch[0b001100] = &CPU::op_andi;
    main_dispatch[0b101000] = &CPU::op_sb;
    main_dispatch[0b100000] = &CPU::op_lb;
    main_dispatch[0b000100] = &CPU::op_beq;
    main_dispatch[0b000111] = &CPU::op_bgtz;
    main_dispatch[0b000110] = &CPU::op_blez;
    main_dispatch[0b100100] = &CPU::op_lbu;
    main_dispatch[0b000001] = &CPU::op_bxx;
    main_dispatch[0b001010] = &CPU::op_slti;
    main_dispatch[0b001011] = &CPU::op_sltiu;
    main_dispatch[0b100101] = &CPU::op_lhu;
    main_dispatch[0b100001] = &CPU::op_lh;
    main_dispatch[0b001110] = &CPU::op_xori;
    main_dispatch[0b100010] = &CPU::op_lwl;
    main_dispatch[0b100110] = &CPU::op_lwr;
    main_dispatch[0b101010] = &CPU::op_swl;
    main_dispatch[0b101110] = &CPU::op_swr;
    main_dispatch[0b110000] = &CPU::op_lwc0;
    main_dispatch[0b110001] = &CPU::op_lwc1;
    main_dispatch[0b110010] = &CPU::op_lwc2;
    main_dispatch[0b110011] = &CPU::op_lwc3;
    main_dispatch[0b111000] = &CPU::op_swc0;
    main_dispatch[0b111001] = &CPU::op_swc1;
    main_dispatch[0b111010] = &CPU::op_swc2;
    main_dispatch[0b111011] = &CPU::op_swc3;

    this->load_reg = 0;
    this->load_val = 0;

    // $0 always zero
    this->set_reg(0, 0);
}

void CPU::run_next_instruction() {
    uint32_t pc = this->program_counter;

    if (pc % 4 != 0) {
        this->exception(Exception::LoadAddressError);
        return;
    }

    Instruction instruction = Instruction(this->load32(pc));

    // delay slot
    this->delay_slot = this->branch_occured;
    this->branch_occured = false;

    this->program_counter = this->next_program_counter;
    this->next_program_counter += 4;

    this->set_reg(this->load_reg, this->load_val);

    this->load_reg = 0;
    this->load_val = 0;

    this->current_program_counter = pc;
    this->execute_instruction(instruction);

    std::copy(std::begin(this->out_regs),
              std::end(this->out_regs), std::begin(this->regs));

    // r3000d_disassemble(buf, instruction.opcode, NULL);
    // printf("%x : %s\n", this->current_program_counter, buf);

    this->opcode_count += 1;
}

uint32_t CPU::load32(uint32_t p_addr) {
    return this->inter->load32(p_addr);
}
uint16_t CPU::load16(uint32_t p_addr) {
    return this->inter->load16(p_addr);
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
        return;
    }

    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    uint32_t v = this->get_reg(t);

    if (addr % 4 == 0) {
        this->store32(addr, v);
    } else {
        this->exception(Exception::StoreAddressError);
    }
}
void CPU::op_swl(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    uint32_t v = this->get_reg(t);

    uint32_t aligned_addr = addr & !3;
    uint32_t cur_mem = this->load32(aligned_addr);

    uint32_t mem = (uint32_t)NULL;

    switch (addr & 3) {
    case 0: {
        mem = (cur_mem & 0xffffff00) | (v >> 24);
        break;
    }
    case 1: {
        mem = (cur_mem & 0xffff0000) | (v >> 16);
        break;
    }
    case 2: {
        mem = (cur_mem & 0xff000000) | (v >> 8);
        break;
    }
    case 3: {
        mem = (cur_mem & 0x00000000) | (v >> 0);
        break;
    }
    }
    this->store32(aligned_addr, mem);
}
void CPU::op_swr(Instruction p_instruction) {

    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    uint32_t v = this->get_reg(t);

    uint32_t aligned_addr = addr & !3;
    uint32_t cur_mem = this->load32(aligned_addr);

    uint32_t mem = (uint32_t)NULL;

    switch (addr & 3) {
    case 0: {
        mem = (cur_mem & 0x00000000) | (v << 0);
        break;
    }
    case 1: {
        mem = (cur_mem & 0x000000ff) | (v << 8);
        break;
    }
    case 2: {
        mem = (cur_mem & 0x0000ffff) | (v << 16);
        break;
    }
    case 3: {
        mem = (cur_mem & 0x00ffffff) | (v << 24);
        break;
    }
    }
    this->store32(aligned_addr, mem);
}
// Store half word
void CPU::op_sh(Instruction p_instruction) {
    if ((this->status_register & 0x10000) != 0) {
        return;
    }
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;
    uint32_t v = this->get_reg(t);

    if (addr % 2 == 0) {
        this->store16(addr, (uint16_t)v);
    } else {
        this->exception(Exception::StoreAddressError);
    }
}
// Store byte
void CPU::op_sb(Instruction p_instruction) {
    if ((this->status_register & 0x10000) != 0) {
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
        return;
    }
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    // Address must be 32bit aligned
    if (addr % 4 == 0) {
        uint32_t v = this->load32(addr);

        this->load_reg = t;
        this->load_val = v;
    } else {
        this->exception(Exception::LoadAddressError);
    }
}
void CPU::op_lwl(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    // This instruction bypasses the load delay restriction: this
    // instruction will merge the new contents with value
    // currently being loaded if need be

    uint32_t cur_v = this->out_regs[t];

    uint32_t aligned_addr = addr & !3;
    uint32_t aligned_word = this->load32(aligned_addr);

    uint32_t v = (uint32_t)NULL;

    switch (addr & 3) {
    case 0: {
        v = (cur_v & 0x00ffffff) | (aligned_word << 24);
        break;
    }
    case 1: {
        v = (cur_v & 0x0000ffff) | (aligned_word << 16);
        break;
    }
    case 2: {
        v = (cur_v & 0x000000ff) | (aligned_word << 8);
        break;
    }
    case 3: {
        v = (cur_v & 0x00000000) | (aligned_word << 0);
        break;
    }
    }
    this->load_reg = t;
    this->load_val = v;
}
void CPU::op_lwr(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    // This instruction bypasses the load delay restriction: this
    // instruction will merge the new contents with value
    // currently being loaded if need be

    uint32_t cur_v = this->out_regs[t];

    uint32_t aligned_addr = addr & !3;
    uint32_t aligned_word = this->load32(aligned_addr);

    uint32_t v = (uint32_t)NULL;

    switch (addr & 3) {
    case 3: {
        v = (cur_v & 0xffffff00) | (aligned_word << 24);
        break;
    }
    case 2: {
        v = (cur_v & 0xffff0000) | (aligned_word << 16);
        break;
    }
    case 1: {
        v = (cur_v & 0xff000000) | (aligned_word << 8);
        break;
    }
    case 0: {
        v = (cur_v & 0x00000000) | (aligned_word << 0);
        break;
    }
    }
    this->load_reg = t;
    this->load_val = v;
}
// Load half word unsigned
void CPU::op_lhu(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    if (addr % 2 != 0) {
        this->exception(Exception::LoadAddressError);
        return;
    }

    uint32_t v = (uint32_t)this->load16(addr);
    this->load_reg = t;
    this->load_val = v;
}

void CPU::op_lh(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    // Cast to i16
    int16_t v = (int16_t)this->load16(addr);

    this->load_reg = t;
    this->load_val = (uint32_t)v;
}
// Load byte
void CPU::op_lb(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    int8_t v = (int8_t)this->load8(addr);

    this->load_reg = t;
    this->load_val = (uint32_t)v;
}

// Load byte unsigned
void CPU::op_lbu(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t t = p_instruction.t();
    uint32_t s = p_instruction.s();

    uint32_t addr = this->get_reg(s) + i;

    uint8_t v = this->load8(addr);

    this->load_reg = t;
    this->load_val = (uint32_t)v;
}

// Shift Left Logical
void CPU::op_sll(Instruction p_instruction) {
    uint32_t i = p_instruction.shift();
    uint32_t t = p_instruction.t();
    uint32_t d = p_instruction.d();

    uint32_t v = this->get_reg(t) << i;

    this->set_reg(d, v);
}

// Shift Left Logical Variable
void CPU::op_sllv(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    // Shift amount is truncated to 5 bits
    uint32_t v = this->get_reg(t) << (this->get_reg(s) & 0x1f);

    this->set_reg(d, v);
}

void CPU::op_sra(Instruction p_instruction) {
    uint32_t i = p_instruction.shift();
    uint32_t d = p_instruction.d();
    uint32_t t = p_instruction.t();

    uint32_t v = (int32_t)this->get_reg(t) >> i;
    this->set_reg(d, (uint32_t)v);
}

void CPU::op_srav(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    // Shift amount is truncated to 5 bits
    int32_t value = (int32_t)(get_reg(t));
    uint32_t shift_amount = get_reg(s) & 0x1F;
    int32_t result = value >> shift_amount;

    set_reg(d, (uint32_t)(result));
}

void CPU::op_srl(Instruction p_instruction) {
    uint32_t i = p_instruction.shift();
    uint32_t t = p_instruction.t();
    uint32_t d = p_instruction.d();

    uint32_t v = this->get_reg(t) >> i;

    this->set_reg(d, v);
}

void CPU::op_srlv(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    // Shift amount is truncated to 5 bits
    uint32_t shift_amount = this->get_reg(s) & 0x1F;
    uint32_t v = this->get_reg(t) >> shift_amount;

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
    this->branch_occured = true;
    uint32_t i = p_instruction.imm_jump();
    this->next_program_counter =
        (this->current_program_counter & 0xf0000000) | (i << 2);
}

void CPU::op_jal(Instruction p_instruction) {
    this->branch_occured = true;
    this->set_reg(31, this->next_program_counter);
    this->op_jmp(p_instruction);
}

void CPU::op_jalr(Instruction p_instruction) {
    this->branch_occured = true;
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();

    uint32_t return_address = this->next_program_counter;
    this->set_reg(d, return_address);

    this->next_program_counter = this->get_reg(s);
}

void CPU::op_jr(Instruction p_instruction) {
    this->branch_occured = true;
    this->next_program_counter =
        this->get_reg(p_instruction.s());
}

void CPU::op_or(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) | this->get_reg(t);

    this->set_reg(d, v);
}

void CPU::op_nor(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = !(this->get_reg(s) | this->get_reg(t));

    this->set_reg(d, v);
}

void CPU::op_xor(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) ^ this->get_reg(t);

    this->set_reg(d, v);
}

void CPU::op_xori(Instruction p_instruction) {
    uint32_t i = p_instruction.imm();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) ^ i;

    this->set_reg(t, v);
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
    case 0b10000: {
        this->op_rfe(p_instruction);
        break;
    }
    default:
        printf("CPU::OP_COP: Unhandled cop0 instruction: %x\n",
               p_instruction.cop_opcode());
        std::terminate();
    }
}

void CPU::op_cop2(Instruction p_instruction) {
    printf("COP2: UNIMPLEMENTED\n");
}

void CPU::op_cop1(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}
void CPU::op_cop3(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}

void CPU::op_lwc0(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}
void CPU::op_lwc1(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}
void CPU::op_lwc2(Instruction p_instruction) {
    printf("Unhandled GTE LWC: 0x%x\n", p_instruction.opcode);
}
void CPU::op_lwc3(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}

void CPU::op_swc0(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}
void CPU::op_swc1(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
}
void CPU::op_swc2(Instruction p_instruction) {
    printf("Unhandled GTE SWC: 0x%x\n", p_instruction.opcode);
}
void CPU::op_swc3(Instruction p_instruction) {
    this->exception(Exception::CoprocessorError);
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
    }
}

void CPU::op_rfe(Instruction p_instruction) {
    // There are other instructions with the same encoding but
    // all
    // are virtual memory related and the Playstation doesn't
    // implement them. Still, let's make sure we're not running
    // buggy code.

    if ((p_instruction.opcode & 0x3f) != 0b010000) {
        throw std::runtime_error("INVALID cop0 instruction");
    }
    // Restore the pre-exception mode by shifting the Interrupt
    // Enable/User Mode stack back to its original position.

    uint32_t mode = this->status_register & 0x3f;
    this->status_register &= !0x3f;
    this->status_register |= mode >> 2;
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
        printf("Unhandled read from cop0r: 0x%x\n", cop_r);
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
        this->branch_occured = true;
        this->branch(i);
    }
}
void CPU::op_beq(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    if (this->get_reg(s) == this->get_reg(t)) {
        this->branch_occured = true;
        this->branch(i);
    }
}

void CPU::op_bgtz(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();

    int32_t v = (int32_t)this->get_reg(s);

    if (v > 0) {
        branch_occured = true;
        this->branch(i);
    }
}
void CPU::op_blez(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();

    int32_t v = (int32_t)this->get_reg(s);

    if (v <= 0) {
        branch_occured = true;
        this->branch(i);
    }
}

void CPU::op_bxx(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();

    uint32_t instr = p_instruction.opcode;

    uint32_t is_bgez = (instr >> 16) & 1;
    uint32_t is_link = ((instr >> 17) & 0xf) == 8;

    int32_t v = (int32_t)this->get_reg(s);

    // test less than zero
    uint32_t test = (uint32_t)(v < 0);
    test = test ^ is_bgez;

    if (is_link) {
        uint32_t return_address = this->next_program_counter;
        this->set_reg(31, return_address);
    }
    if (test != 0) {
        this->branch_occured = true;
        this->branch(i);
    }
}
void CPU::op_addi(Instruction instruction) {
    uint32_t i = instruction.imm_se();
    uint32_t t = instruction.t();
    uint32_t s = instruction.s();
    uint32_t s_val = this->get_reg(s);

    int32_t result;
    if (__builtin_add_overflow((int32_t)s_val, (int32_t)i,
                               &result)) {
        this->exception(Exception::ArithmeticOverflow);
        return;
    }
    this->set_reg(t, (uint32_t)result);
}

void CPU::op_add(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t d = p_instruction.d();
    uint32_t t = p_instruction.t();

    int32_t sv = (int32_t)this->get_reg(s);
    int32_t tv = (int32_t)this->get_reg(t);

    int32_t result;
    if (__builtin_add_overflow(sv, tv, &result)) {
        this->exception(Exception::ArithmeticOverflow);
        return;
    }
    this->set_reg(d, (uint32_t)result);
}

void CPU::op_sub(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t d = p_instruction.d();
    uint32_t t = p_instruction.t();

    int32_t si = (int32_t)this->get_reg(s);
    int32_t ti = (int32_t)this->get_reg(t);

    int32_t result;
    if (__builtin_sub_overflow(si, ti, &result)) {
        this->exception(Exception::ArithmeticOverflow);
        return;
    }
    this->set_reg(d, (uint32_t)result);
}

void CPU::op_subu(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();
    uint32_t d = p_instruction.d();

    uint32_t v = this->get_reg(s) - this->get_reg(t);
    this->set_reg(d, v);
}

void CPU::op_div(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    int32_t n = (int32_t)this->get_reg(s);
    int32_t d = (int32_t)this->get_reg(t);

    if (d == 0) {
        this->hi = (uint32_t)n;
        if (n >= 0) {
            this->lo = 0xffffffff;
        } else {
            this->lo = 1;
        }
    } else if ((uint32_t)n == 0x80000000 and d == -1) {
        // Result is not representable in a 32 bit signed integer
        this->hi = 0;
        this->lo = 0x80000000;
    } else {
        this->hi = (uint32_t)(n % d);
        this->lo = (uint32_t)(n / d);
    }
}

void CPU::op_divu(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t n = this->get_reg(s);
    uint32_t d = this->get_reg(t);

    if (d == 0) {
        // Division by zero, results are bogus
        this->hi = n;
        this->lo = 0xffffffff;
    } else {
        this->hi = n % d;
        this->lo = n / d;
    }
}

void CPU::op_mult(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    int64_t a = int64_t(int32_t(this->get_reg(s)));
    int64_t b = int64_t(int32_t(this->get_reg(t)));

    uint64_t v = uint64_t(a * b);

    this->hi = uint32_t(v >> 32);
    this->lo = uint32_t(v);
}

void CPU::op_multu(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint64_t a = this->get_reg(s);
    uint64_t b = this->get_reg(t);

    uint64_t v = a * b;

    this->hi = uint32_t(v >> 32);
    this->lo = uint32_t(v);
}

void CPU::op_mflo(Instruction p_instruction) {
    uint32_t d = p_instruction.d();

    uint32_t lo = this->lo;

    this->set_reg(d, lo);
}
void CPU::op_mfhi(Instruction p_instruction) {
    uint32_t d = p_instruction.d();

    uint32_t hi = this->hi;

    this->set_reg(d, hi);
}

void CPU::op_mtlo(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    this->lo = this->get_reg(s);
}
void CPU::op_mthi(Instruction p_instruction) {
    uint32_t s = p_instruction.s();
    this->hi = this->get_reg(s);
}

void CPU::op_stlu(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) < this->get_reg(t);
    this->set_reg(d, v);
}

void CPU::op_slti(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) < i;

    this->set_reg(t, v);
}
void CPU::op_sltiu(Instruction p_instruction) {
    uint32_t i = p_instruction.imm_se();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) < i;

    this->set_reg(t, v);
}
void CPU::op_slt(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    int32_t sv = this->get_reg(s);
    int32_t tv = this->get_reg(t);

    uint32_t v = sv < tv;
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

void CPU::op_and(Instruction p_instruction) {
    uint32_t d = p_instruction.d();
    uint32_t s = p_instruction.s();
    uint32_t t = p_instruction.t();

    uint32_t v = this->get_reg(s) & this->get_reg(t);

    this->set_reg(d, v);
}

void CPU::branch(uint32_t p_offset) {
    uint32_t offset = p_offset << 2;
    this->next_program_counter =
        this->current_program_counter + offset + 4;
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

    if (this->delay_slot) {
        this->epc_register = this->epc_register + 4;
        this->cause_register |= 1 << 31;
    }

    this->program_counter = handler;
    this->next_program_counter = this->program_counter + 4;
}

void CPU::op_syscall(Instruction) {
    this->exception(Exception::SysCall);
}

void CPU::op_break(Instruction) {
    this->exception(Exception::Break);
}

void CPU::op_illegal(Instruction p_instruction) {
    printf("CPU::DECODE: Unhandled instruction: 0x%x\n",
           p_instruction.opcode);
    this->exception(Exception::IllegalInstruction);
}

void CPU::execute_instruction(Instruction p_instruction) {
    uint32_t function = p_instruction.function();
    uint32_t subfunction = p_instruction.subfunction();

    if (function == 0x0) {
        op_handler h = rtype_dispatch[subfunction];
        if (h)
            (this->*h)(p_instruction);
        else
            this->op_illegal(p_instruction);

        return;
    }
    op_handler h = main_dispatch[function];
    if (h)
        (this->*h)(p_instruction);
    else
        this->op_illegal(p_instruction);
}
void CPU::run() {
    while (true) {
        this->run_next_instruction();
    }
}
