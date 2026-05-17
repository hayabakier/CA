#include "defs.h"

void fetch(int cycle)
{
    short int instruction = instructionMemory[pc];
    short int ipc = pc;
    pc++;

    IF_ID.valid = 1;
    IF_ID.pc_of_instr = ipc;
    IF_ID.raw = instruction;

    printf("[IF  | cycle %d] PC=%d  encoded=%d\n",
           cycle, IF_ID.pc_of_instr, instruction);
}

void decode(int cycle)
{
    if (!IF_ID.valid)
    {
        printf("[ID  | cycle %d] (bubble)\n", cycle);
        ID_EX.valid = 0;
        return;
    }

    short int instruction = IF_ID.raw;
    IF_ID.opcode = ((instruction >> 12) & 0b1111);
    IF_ID.r1 = (instruction >> 6) & 0b111111;
    IF_ID.r2 = instruction & 0b111111;

    int raw_imm = instruction & 0b111111;
    IF_ID.imm = (raw_imm & 0b100000) ? (int8_t)(raw_imm | 0b11000000) : (int8_t)raw_imm;

    IF_ID.val_r1 = registerFile[IF_ID.r1];
    IF_ID.val_r2 = registerFile[IF_ID.r2];

    detectHazard();

    if (stall_pipeline)
    {
        printf("[ID  | cycle %d] ** STALL ** holding R%d for load-use hazard\n",
               cycle, IF_ID.r1);
        ID_EX.valid = 0;

        return;
    }

    ID_EX = IF_ID;
    memset(&IF_ID, 0, sizeof(Instruction));
    ID_EX.valid = 1;

    ID_EX.val_r1 = registerFile[ID_EX.r1];
    ID_EX.val_r2 = registerFile[ID_EX.r2];

    applyForwarding(&ID_EX, cycle);

    int itype = (ID_EX.opcode == LDI || ID_EX.opcode == BEQZ ||
                 ID_EX.opcode == SAL || ID_EX.opcode == SAR ||
                 ID_EX.opcode == LB || ID_EX.opcode == SB);

    if (itype)
        printf("[ID  | cycle %d] PC=%d  opcode=%d  R%d(%d)  imm=%d\n",
               cycle, ID_EX.pc_of_instr, ID_EX.opcode,
               ID_EX.r1, (int)ID_EX.val_r1,
               (int)ID_EX.imm);
    else
        printf("[ID  | cycle %d] PC=%d  opcode=%d  R%d(%d)  R%d(%d)\n",
               cycle, ID_EX.pc_of_instr, ID_EX.opcode,
               ID_EX.r1, (int)ID_EX.val_r1,
               ID_EX.r2, (int)ID_EX.val_r2);
}