/* memory.c — owns the two memory arrays (no logic) */
#include "defs.h"

short int instructionMemory[INSTR_MEM_SIZE] = {0};
int8_t    dataMemory[DATA_MEM_SIZE]         = {0};
int       NumberofInstructions              = 0;
