#ifndef DEBUG_RUN_H
#define DEBUG_RUN_H

#include <string>

#include "lc3.h"
#include "lc3_debug.h"

int debug_loop(LC3_Debugger *machine);

int handle_break(LC3_Debugger &machine);

std::string check_command(LC3_Debugger &machine, const std::string &command, const std::string &prev = "");

std::string handle_invalid_input(LC3_Debugger &machine, const std::string &prev);

#endif