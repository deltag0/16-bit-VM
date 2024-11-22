// debug should allow users to type break and a memory address
// check if it's a valid memory address
// can provide warning if memory hasn't been accessed yet
// provide step function
// provide continue function which runs program until we reach a memory address that's in the breakpoints
// mashallah habdummila

#include <iostream>

#include "lc3_debug.h"

void LC3_Debugger::print_addr() {
    uint16_t flag = reg[R_COND];
    std::cout << "Flag is currently ";

    if (flag & 0x1) {
        std::cout << "positive: ";
    }
    else if (flag & 0x2) {
        std::cout << "zero: ";
    }
    else if (flag & 0x4) {
        std::cout << "negative: ";
    }
    std::cout << "0x" << std::hex << flag << '\n';

    std::cout << "Currently at address: " << reg[R_PC] << '\n';
}