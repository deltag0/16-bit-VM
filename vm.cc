#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>
#include <signal.h>
#include <cstdint>

#include "lc3.h"
#include "lc3_run.h"

#include "lc3_debug.h"
#include "debug_run.h"

using std::string;

void read_image_file(std::ifstream &ifs, LC3_Machine *machine) {
    uint16_t origin;

    // big endian to little endian
    ifs.read(reinterpret_cast<char *>(&origin), sizeof(origin));

    swap16(origin); // Swap is needed if the input is in big endian, then we want to convert it into little endian

    uint16_t *max_read = machine->memory + MEMORY_MAX - 1;
    uint16_t *p = machine->memory + origin;

    while (p != max_read && ifs) {
        ifs.read(reinterpret_cast<char *>(p), sizeof(origin));
        swap16(*p); // same as above
        p++;
    }
}

int read_image(std::ifstream &ifs, LC3_Machine *machine) {
    if (!ifs) {
        throw std::runtime_error("Invalid File provided");
    }

    read_image_file(ifs, machine);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        throw std::runtime_error("Not enough arguments provided (image is probably missing)");
    }

    bool debug_mode = false;

    // declare both types, in order to assign type to corresponding machine
    // machine will always be initialized, only it must be deleted.
    LC3_Machine *machine = nullptr;
    LC3_Debugger *debugger = nullptr;

    string file_name = argv[1];

    if (argc == 3) {
        string mode_string = argv[2];

        if (mode_string != "-debug") {
            throw std::runtime_error("Invalid debug mode provided. Available commands are: -debug" );
        }
        else debug_mode = true;
    }

    if (debug_mode) {
        machine = new LC3_Debugger;
    }
    else {
        machine = new LC3_Machine;
    }

    std::ifstream ifs{file_name, std::ios::binary};

    read_image(ifs, machine);

    // since exactly one condition flag should be set at any given time, set the Z flag
    (*machine).reg[R_COND] = FL_ZRO;

    // set the PC to starting position 
    enum { PC_START = 0x3000 };
    machine->reg[R_PC] = PC_START;

    int running = 1;
    if (debug_mode) {
        debugger = static_cast<LC3_Debugger*>(machine);

        string command;
        while (command != "run") {
            std::cout << '>';
            std::getline(std::cin, command);

            // needa make sure we handle adding commands after initial command.
            // like: >run step break 1000. Ignore everything after
            command = check_command(*debugger, command);
        }
        debugger->running = true;
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    while (running) {
        if (!debug_mode) {
            running = run_loop(machine);
        }
        else {
            // casting pointer here isn't great
            // could have a virtual clone function to determine type at runtime
            // could have different handling with if statements
            // parent interface class (but both classes would need same methods)
            running = debug_loop(debugger);
        }
    }

    delete machine;
    restore_input_buffering();
}



/*
The LC-3 supports five addressing modes: immediate (or literal), register,
and three memory addressing modes: PC-relative, indirect, and Base+offset.

 Each time a GPR is written by an operate
or a load instruction, the N, Z, and P one-bit registers are individually set to 0 or 1

only modifying GPR

PC is incremented by 1 every time we execute something


ADD AND NOT, LEA, LD, SD, LDI, SDI, LDR

All L's and S's require 
offsets (They are load / store instructions)

Load -> load something in registers
Store -> store something in memory

LEA (requires registry): adds current value at PC with the sign extended value in its instruction into a register it specified


LD (requires registyr): Gets address PC is pointing to and increments it with 8bit value specified in instruction, and loads the value at that address in the registry specified
SD -> Opposite of LD (stores value of register in memory location specified)


LDI (requires registry): same thing as LD, but instead of loading the value from the offset, the value at the address is another address. It gets the value from that address. (should be a meaningful address)
STI -> Get address1 by adding offset to PC. Then treat the value of address1 as address2. Store content of SR (source registry) in address2

LDR (requires 2 registris R1, R2, offset): adds addresses of offset and R2, and stores the value of the added address into R1
STR -> Same thing as LDR but stores the value in SR (R1) into the second registry + offset

Control functions:

BR: 
Have 3 bits for (N, P, Z) somewhere in the middle of the instruction. 

If our N (negative), P (positive) and Z (zero) registries match at least one of the 3 bits we branch to the PC + offset

JMP:

Only has a registry address, and changes JMP directly to the value in the specified registry (should be an address)


TRAP:
request that the operating system performs a task for us (service call)




*/