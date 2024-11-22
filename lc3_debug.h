#ifndef LC3_DEBUG_H
#define LC3_DEBUG_H

#include "lc3.h"
#include <unordered_set>
#include "lc3_run.h"

struct LC3_Debugger: public LC3_Machine {
    // breakpoints
    int num_breakpoints;
    std::unordered_set<uint16_t> breakpoints;

    // used for commands before running and before having ran
    bool running = false;

    // print current R_PC address
    void print_addr();

};


#endif
