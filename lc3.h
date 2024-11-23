#ifndef LC3_H
#define LC3_H

#include <cstdint>
#define MEMORY_MAX (1 << 16)


enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,  // registry 6 is used as a pointer to the "stack" for recursion. The stack starts at the end of the memory, i.e it starts at 0xFFFF
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};
enum {
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

enum {
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum {
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

enum {
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

class lc3_instruction {
    uint16_t data;

    public:
        lc3_instruction(uint16_t data);
        uint16_t opcode() const;
        uint16_t operands() const;

        uint16_t dr() const;
        uint16_t sr1() const;
        uint16_t base_r() const;
        uint16_t sr2() const;
        bool is_imm() const;
        uint16_t imm5() const;

        uint16_t offset6() const;
        uint16_t pc_offset9() const;

        bool is_jsr() const;
        uint16_t pc_offset11() const;

        // TRAP
        uint16_t vector() const;

        bool n() const;
        bool z() const;
        bool p() const;
        uint16_t cc() const;

        uint16_t get(int i, int length) const;
        uint16_t bits() const;
};

uint16_t sign_extend(uint16_t x, int bit_count);

void swap16(uint16_t &x);

struct LC3_Machine {
    uint16_t memory[MEMORY_MAX];  /* 65536 locations */
    uint16_t reg[R_COUNT];
    uint16_t depth;
    uint16_t counter;
    
    
    bool debug = false;

    virtual ~LC3_Machine() = default;

};

void update_flags(uint16_t r, LC3_Machine *machine);

#endif