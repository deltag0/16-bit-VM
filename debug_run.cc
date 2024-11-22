#include <string>
#include <iostream>
#include <sstream>

#include "debug_run.h"
#include "lc3_run.h"

using std::string;

int debug_loop(LC3_Debugger *machine) {
    if (machine->breakpoints.find(machine->reg[R_PC]) != machine->breakpoints.end()) {
        machine->print_addr();
        return handle_break(*machine);
    }

    return run_loop(machine, false);
}


int handle_break(LC3_Debugger &machine) {
    string prev = "";
    string curr = "step";
    int running = 1;

    // probably make this a do while loop
    while (curr == "next" || curr == "n" || curr == "step" || curr == "s") {
        restore_input_buffering();
        std::cout << '>';
        prev = curr;

        std::getline(std::cin, curr);

        curr = check_command(machine, curr, prev);

        if (curr == "step" || curr == "s") {
            running = run_loop(&machine, true);
            machine.print_addr();
        }
        else if (curr == "continue" || curr == "c") {
            disable_input_buffering();
            break;
        }
        else {
            curr = "step";
            continue;
        }
        // since now we have the check_command, that function handles invalid input, so if we don't have a command we must handle
        // in handle_break, we leave it to check_command

        if (running == 0) {
            std::cout << "Program terminated at address " <<  machine.reg[R_PC];
            lc3_instruction last_instr{mem_read(machine.reg[R_PC], &machine)};
            if (last_instr.opcode() == TRAP_HALT) {
                std::cout << "Program terminated successfully" << '\n';
            }
            else {
                std::cout << "Program terminated with errors" << '\n';
            }
        }
    }
    return running;
}

// make sure this throws an error we catch for invalid input
string check_command(LC3_Debugger &machine, const string &command, const string &prev) {
    std::istringstream iss{command};
    string first = "";

    iss >> first;

    if (first == "") first = prev;

    // all commands that can be run anytime
    if (first == "break") {
        uint16_t addr = 0;
        machine.num_breakpoints++;

        iss >> std::hex >> addr;
        
        if (machine.breakpoints.find(addr) != machine.breakpoints.end()) std::cout << "Breakpoint at 0x" << std::hex << addr << " already exists" << '\n';
        else {
        std::cout << "Breakpoint " << machine.num_breakpoints << " at address 0x" << std::hex << addr << '\n';
        machine.breakpoints.insert(addr);
        }

        return first;
    }

    // all commands when machine.running = true
    if (machine.running) {
        if (first == "step" || first == "s") ;
        else if (first == "continue" || first == "c") ;
        else if (first == "next" || first == "n") ;
        // need to add all commands these are all for now

        else {
            return handle_invalid_input(machine, first);
        }
    }
    // only check for run the first time anyways, so do we even need this?
    else if (first == "run") ;
    else {
        return handle_invalid_input(machine, first);
    }

    return first;
}

string handle_invalid_input(LC3_Debugger &machine, const string &prev) {
    string new_command;
    std::cout << '"' << prev << '"' << " Is not a valid command." << '\n';

    std::cout << '>';

    std::getline(std::cin, new_command);

    return check_command(machine, new_command);
}
