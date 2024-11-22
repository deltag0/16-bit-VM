#include "lc3.h"

#include <cstdint>

lc3_instruction::lc3_instruction(uint16_t data): data{data} {}

uint16_t lc3_instruction::opcode() const {return data >> 12 & 0xF;} // opcode only
uint16_t lc3_instruction::operands() const {return data & 0xFFF;} // everything except the opcode
uint16_t lc3_instruction::dr() const {return (data >> 9) & 0x7;}
uint16_t lc3_instruction::sr1() const {return (data >> 6) & 0x7;}
uint16_t lc3_instruction::base_r() const {return (data >> 6) & 0x7;}
uint16_t lc3_instruction::sr2() const {return data & 0x7;}
bool lc3_instruction::is_imm() const {return (data >> 5) & 0x1;}
uint16_t lc3_instruction::imm5() const {return sign_extend(data & 0x1F, 5);}

uint16_t lc3_instruction::offset6() const {return sign_extend(data & 0x3F, 6);}
uint16_t lc3_instruction::pc_offset9() const {return sign_extend(data & 0x1FF, 9);}

bool lc3_instruction::is_jsr() const {return (data >> 11) & 0x1;}
uint16_t lc3_instruction::pc_offset11() const {return sign_extend(data & 0x7FF, 11);}

// TRAP
uint16_t lc3_instruction::vector() const {return data & 0xFF;}

bool lc3_instruction::n() const {return (data >> 11) & 0x1;}
bool lc3_instruction::z() const {return (data >> 10) & 0x1;}
bool lc3_instruction::p() const {return (data >>  9) & 0x1;}
uint16_t lc3_instruction::cc() const {return (data >> 9) & 0x7;}

uint16_t lc3_instruction::get(int i, int length) const {return (data >> i) & ((1 << length) - 1);} // -1 flips bits behind the 1
uint16_t lc3_instruction::bits() const {return data;}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void swap16(uint16_t &x) {
    x = (x << 8) | (x >> 8);
}

void update_flags(uint16_t r, LC3_Machine *machine) {
    if (machine->reg[r] == 0)
    {
        machine->reg[R_COND] = FL_ZRO;
    }
    else if (machine->reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        machine->reg[R_COND] = FL_NEG;
    }
    else
    {
        machine->reg[R_COND] = FL_POS;
    }
}