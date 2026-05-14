#ifndef MEMORY_WB_H
#define MEMORY_WB_H

#include <stdint.h>
#include "pipeline.h"

extern int8_t fwd_result;
extern int    fwd_dest_reg;
extern int    fwd_valid;

void memAccess(int opcode, int r1, int8_t imm);
void writeBack(int r1, int8_t result);
void applyForwarding(Instruction *id_ex_latch);
void printCycleState(int cycle);

#endif