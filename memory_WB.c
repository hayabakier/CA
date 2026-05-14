#include <stdio.h>
#include <stdint.h>
#include "memory_wb.h"
#include "memory.h"
#include "pipeline.h"
#include "parser.h"

extern int8_t registerFile[64];
int8_t fwd_result   = 0;
int    fwd_dest_reg = -1;
int    fwd_valid    = 0;

void memAccess(int opcode, int registerindex, int8_t imm)
{
    int address = imm & 0x3F;

    if (opcode == LB)
    {
        int8_t loadeddata = dataMemory[address];

        if (registerindex != 0)
        {
            registerFile[registerindex] = loadeddata;
            printf("LB: loaded dataMemory[%d] = %d into R%d\n", address, (int)loadeddata, registerindex);
        }
        else
        {
            printf("LB: loaded dataMemory[%d] = %d but R0 stays 0\n", address, (int)loadeddata);
        }

        fwd_result   = loadeddata;
        fwd_dest_reg = registerindex;
        fwd_valid    = 1;
    }
    else if (opcode == SB)
    {
        int8_t storeddata = registerFile[registerindex];
        dataMemory[address] = storeddata;       
        printf("SB: stored R%d = %d into dataMemory[%d]\n", registerindex, (int)storeddata, address);
        fwd_valid    = 0;
        fwd_dest_reg = -1;
    }
}

void writeBack (int registerindex, int8_t result)
{
    if (registerindex == 0)
    {
        printf("WB: result = %d but R0 stays 0\n", (int)result);
    }
    else
    {
        registerFile[registerindex] = result;
        printf("WB: writing %d into R%d\n", (int)result, registerindex);
    }

    fwd_result   = result;
    fwd_dest_reg = registerindex;
    fwd_valid    = 1;
}

void applyForwarding(Instruction *id_ex_latch)
{
    if (!fwd_valid || fwd_dest_reg <= 0)
        return;

    int opcode   = (*id_ex_latch).opcode;
    int reads_r1 = 0;
    int reads_r2 = 0;

    switch (opcode)
    {
        case ADD: case SUB: case MUL: case AND: case OR: case JR:
            reads_r1 = 1; reads_r2 = 1; break;
        case LDI: case SAL: case SAR: case LB: case SB: case BEQZ:
            reads_r1 = 1; reads_r2 = 0; break;
        default: break;
    }

    if (reads_r1 && ((*id_ex_latch).r1 == fwd_dest_reg))
    {
        printf("Forwarding: R%d has stale value %d, replacing with %d\n", (*id_ex_latch).r1, (int)(*id_ex_latch).val_r1, (int)fwd_result);
        (*id_ex_latch).val_r1 = fwd_result;
    }

    if (reads_r2 && ((*id_ex_latch).r2 == fwd_dest_reg))
    {
        printf("Forwarding: R%d has stale value %d, replacing with %d\n", (*id_ex_latch).r2, (int)(*id_ex_latch).val_r2, (int)fwd_result);
        (*id_ex_latch).val_r2 = fwd_result;
    }
}

void printCycleState(int cycle)
{
    printf("  --- Cycle %d Latch State ---\n", cycle);

    if (IF_ID.valid)
        printf("  [STATE]  IF/ID : PC=%d  opcode=%d  R%d  R%d  imm=%d\n",
               IF_ID.pc_of_instr, IF_ID.opcode,
               IF_ID.r1, IF_ID.r2, (int)IF_ID.imm);
    else
        printf("  [STATE]  IF/ID : (bubble)\n");

    if (ID_EX.valid)
        printf("  [STATE]  ID/EX : PC=%d  opcode=%d  R%d(%d)  R%d(%d)  imm=%d\n",
               ID_EX.pc_of_instr, ID_EX.opcode,
               ID_EX.r1, (int)ID_EX.val_r1,
               ID_EX.r2, (int)ID_EX.val_r2,
               (int)ID_EX.imm);
    else
        printf("  [STATE]  ID/EX : (bubble)\n");
}