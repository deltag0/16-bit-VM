#include <iostream>
#include <cstdint>
#include <bitset>
#include <signal.h>
#include <Windows.h>
#include <conio.h>
#include <stdexcept>

#include "lc3.h"
#include "lc3_run.h"

HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

void disable_input_buffering() {
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode); /* save old mode */
    fdwMode = fdwOldMode
            ^ ENABLE_ECHO_INPUT  /* no input echo */
            ^ ENABLE_LINE_INPUT; /* return when one or
                                    more characters are available */
    SetConsoleMode(hStdin, fdwMode); /* set new mode */
    FlushConsoleInputBuffer(hStdin); /* clear buffer */
}

void restore_input_buffering() {
    SetConsoleMode(hStdin, fdwOldMode);
}

uint16_t check_key() {
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

void mem_write(uint16_t address, uint16_t val, LC3_Machine *machine)
{
    machine->memory[address] = val;
}

uint16_t mem_read(uint16_t address, LC3_Machine *machine)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            machine->memory[MR_KBSR] = (1 << 15);
            machine->memory[MR_KBDR] = getchar();
        }
        else
        {
            machine->memory[MR_KBSR] = 0;
        }
    }
    return machine->memory[address];
}

int run_loop(LC3_Machine *machine, bool debug) {
    /* FETCH */
    uint16_t *reg = machine->reg;
    lc3_instruction instr{mem_read(machine->reg[R_PC]++, machine)}; // this is fine because we're R_PC doesn't mean anything. The actual R_PC address is what's in the registry
    uint16_t op = instr.opcode();


    switch (op) {
        case OP_ADD: {
            if (debug) {
                std::cout << "Executing operation: ADD\n";
            }
            if (instr.is_imm()) {
                reg[instr.dr()] = reg[instr.sr1()] + instr.imm5();
            }
            else {
                reg[instr.dr()] = reg[instr.sr1()] + reg[instr.sr2()];
            }
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_AND: {
            if (debug) {
                std::cout << "Executing operation: AND\n";
            }
            if (instr.is_imm()) {
                reg[instr.dr()] = reg[instr.sr1()] & instr.imm5();
            }
            else {
                reg[instr.dr()] = reg[instr.sr1()] & reg[instr.sr2()];
            }
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_NOT: {
            if (debug) {
                std::cout << "Executing operation: NOT\n";
            }
            reg[instr.dr()] = ~reg[instr.sr1()];
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_BR: {
            if (debug) {
                std::cout << "Executing operation: BR\n";
            }
            uint16_t p = (instr.bits() >> 9) & 0x7;
            if (p & reg[R_COND]) {
                reg[R_PC] += instr.pc_offset9();
            }
            break;
        }

        case OP_JMP: {
            if (debug) {
                std::cout << "Executing operation: JMP\n";
            }

            if (instr.base_r() == 0x7) {
                uint16_t addr = pop(machine);
                // set PC to addr
            }

            reg[R_PC] = reg[instr.base_r()];
            break;
        }

        case OP_JSR: {
            if (debug) {
                std::cout << "Executing operation: JSR\n";
            }

            if (machine->depth > 0) {
                // function called inside of functio, so we save the counter on top of stack
                push(machine->counter, machine);
                machine->counter = 0;
            }

            machine->depth++;

            push(reg[R_PC], machine);
            reg[R_R7] = reg[R_PC];
            if (instr.is_jsr()) {
                reg[R_PC] += instr.pc_offset11();
            }
            else {
                reg[R_PC] = reg[instr.base_r()];
            }
            break;
        }

        case OP_LD: {
            if (debug) {
                std::cout << "Executing operation: LD\n";
            }
            reg[instr.dr()] = mem_read(reg[R_PC] + instr.pc_offset9(), machine);
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_LDI: {
            if (debug) {
                std::cout << "Executing operation: LDI\n";
            }
            reg[instr.dr()] = mem_read(mem_read(reg[R_PC] + instr.pc_offset9(), machine), machine);
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_LDR: {
            if (debug) {
                std::cout << "Executing operation: LDR\n";
            }
            reg[instr.dr()] = mem_read(reg[instr.base_r()] + instr.offset6(), machine);
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_LEA: {
            if (debug) {
                std::cout << "Executing operation: LEA\n";
            }
            reg[instr.dr()] = reg[R_PC] + instr.pc_offset9();
            update_flags(instr.dr(), machine);
            break;
        }

        case OP_ST: {
            if (debug) {
                std::cout << "Executing operation: ST\n";
            }

            if (machine->depth > 0) {
                push(reg[instr.dr()], machine);
            }

            mem_write(reg[R_PC] + instr.pc_offset9(), reg[instr.dr()], machine);
            break;
        }

        case OP_STI: {
            if (debug) {
                std::cout << "Executing operation: STI\n";
            }

            if (machine->depth > 0) {
                push(reg[instr.dr()], machine);
            }

            mem_write(mem_read(reg[R_PC] + instr.pc_offset9(), machine), reg[instr.dr()], machine);
            break;
        }
                
        case OP_STR: {
            if (debug) {
                std::cout << "Executing operation: STR\n";
            }

            if (machine->depth > 0) {
                push(reg[instr.dr()], machine);
            }

            mem_write(reg[instr.base_r()] + instr.offset6(), reg[instr.dr()], machine);
            break;
        }

        case OP_TRAP: {
            reg[R_R7] = reg[R_PC];
            switch (instr.bits() & 0xFF) {
                case TRAP_GETC: {
                    char c;

                    std::cin >> c;
                    // while (!(std::cin >> c)) {
                    //     std::cerr << "Invalid input\n";
                    //     std::cin.clear();
                    //     std::cin.ignore();
                    // }

                    reg[R_R0] = (uint16_t)c;
                    update_flags(R_R0, machine);

                    break;
                }

                case TRAP_OUT: {
                    std::cout << (char)reg[R_R0];
                    break;
                }

                case TRAP_PUTS: {
                    uint16_t *c = machine->memory + reg[R_R0];

                    while (*c != 0x0000) {
                        char curr = *c;

                        std::cout << curr;
                        c++;
                    }

                    break;
                }

                case TRAP_IN: {
                    std::cout << "Enter a character" << std::endl;
                    char c;

                    std::cin >> c;
                    // while (!(std::cin >> c)) {
                    //     std::cerr << "Invalid input" << std::endl;
                    //     std::cin.clear();
                    //     std::cin.ignore();
                    // }
                    std::cout << c << '\n';
                    reg[R_R0] = (uint16_t)c;
                    update_flags(R_R0, machine);

                    break;
                }

                case TRAP_PUTSP: {
                    uint16_t *c = machine->memory + reg[R_R0];
                    // assume each memory address stores 2 characters. 1 character in 1 byte, like in modern systems

                    while (*c != 0x0000) {
                        char char1 = (*c) & 0xFF;
                        char char2 = (*c) >> 8;
                        std::cout << char1;
                        if (char2) std::cout << char2;
                        c++;
                    }

                    break;
                }

                case TRAP_HALT:
                    std::cout << "HALT" << std::endl;
                    return 0;
                    break;
            }


            break;
        }

        case OP_RES:
        case OP_RTI:
        default:
            throw std::runtime_error("Bad Instruction");
            break;
    }
    return 1;
}


void push(uint16_t val, LC3_Machine *machine) {
    mem_write(machine->reg[R_R6], val, machine);
    machine->counter++;
    machine->reg[R_R6]--;
}

uint16_t pop(LC3_Machine *machine) {
    machine->reg[R_R6]--;
    uint16_t return_addr = 0x0000;

    while (machine->counter > 0) {
        if (machine->counter == 1) {
            return_addr = mem_read(machine->reg[R_R6], machine);
        }
        mem_write(machine->reg[R_R6], 0x0000, machine);
        machine->counter--;
        machine->reg[R_R6]--;
    }

    machine->depth--;

    if (machine->depth > 0) {
        machine->counter = mem_read(machine->reg[R_R6], machine);
        mem_write(machine->reg[R_R6], 0x0000, machine);
    }

    return return_addr;
}

// plan for stack
/*

stack is stored on the memory from:
0xFD00 to 0xF800 or 0xF000

have a counter in the LC3 that keeps track of how many functions deep we're in, and once we're back to 0, we know we're in main (standard flow)

Then, if we call a function inside a function, we store our current count to the top of the stack
When the next function ends, and we need to jump to R7, IF our current count is greater than 0, we retrieve our previous count, and keep going

Need to know that R7 points to the return address of the function
So once we detect that we jump to R7, we know we need to POP something from the stack

The JSR function is the one that calls our function, and stores the return address in R7
So, when JSR is called, we PUSH to the stack. 
We can keep a counter of how many things are pushed during JSR and append that after the function jumps to R7

probably need to make PUSH & POP functions
*/