#ifndef LC3_RUN_H
#define LC3_RUN_H

#include <cstdint>
#include <bitset>
#include <signal.h>
#include <Windows.h>
#include <conio.h>
#include "lc3.h"

const int STACK_END = 0xF800;

void disable_input_buffering();

void restore_input_buffering();

uint16_t check_key();

void handle_interrupt(int signal);

void mem_write(uint16_t address, uint16_t val, LC3_Machine *machine);

uint16_t mem_read(uint16_t address, LC3_Machine *machine);

int run_loop(LC3_Machine *machine, bool debug = false);

void push(uint16_t val, LC3_Machine *machine);

uint16_t pop(LC3_Machine *machine);

#endif