#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdint.h>

void execute(int8_t opcode, int8_t r1, int8_t r2, int8_t imm, uint16_t instructionPC);

#endif