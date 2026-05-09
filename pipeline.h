#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdint.h>

typedef struct
{
    int valid; // 0 if empty/bubble, 1 if holding an instruction
    int pc_of_instr;
    int opcode;
    int r1;
    int r2;
    int8_t imm;
    int8_t val_r1; // registerFile[r1] read in ID
    int8_t val_r2; // registerFile[r2] read in ID
} Instruction;

/* IF_ID : latch between Fetch  and Decode  */
/* ID_EX : latch between Decode and Execute */
extern Instruction IF_ID; // fetch latch
extern Instruction ID_EX; // decode latch

// pipeline function
void runPipeline(void);

#endif
