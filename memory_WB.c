
#include "defs.h"

void applyForwarding(Instruction *latch, int cycle)
{
    if (!fwd_valid || fwd_dest_reg < 0)
        return;

    int reads_r1 = 0, reads_r2 = 0;
    switch (latch->opcode)
    {
    case ADD:
    case SUB:
    case MUL:
    case AND:
    case OR:
    case JR:
        reads_r1 = 1;
        reads_r2 = 1;
        break;
    case LDI:
    case SAL:
    case SAR:
    case LB:
    case BEQZ:
    case SB:
        reads_r1 = 1;
        reads_r2 = 0;
        break;
    default:
        break;
    }

    if (reads_r1 && latch->r1 == fwd_dest_reg)
    {
        printf("[FWD | cycle %d] R%d: stale=%d -> forwarded=%d\n",
               cycle, latch->r1, (int)latch->val_r1, (int)fwd_result);
        latch->val_r1 = fwd_result;
    }
    if (reads_r2 && latch->r2 == fwd_dest_reg)
    {
        printf("[FWD | cycle %d] R%d: stale=%d -> forwarded=%d\n",
               cycle, latch->r2, (int)latch->val_r2, (int)fwd_result);
        latch->val_r2 = fwd_result;
    }
}

// printCycleState — summary of latch contents at end of each cycle.

void printCycleState(int cycle)
{
    printf("  --- End of Cycle %d ---\n", cycle);
    int itype = (IF_ID.opcode == LDI || IF_ID.opcode == BEQZ ||
                 IF_ID.opcode == SAL || IF_ID.opcode == SAR ||
                 IF_ID.opcode == LB || IF_ID.opcode == SB);
    if (IF_ID.valid)
        printf("   [IF  | cycle %d] PC=%d  encoded=%d\n",
               cycle, IF_ID.pc_of_instr, IF_ID.raw);

    else
        printf("  [LATCH IF/ID] (bubble)\n");

    if (ID_EX.valid)
        if (itype)
            printf("  [LATCH ID/EX] PC=%d  opcode=%d  R%d(%d)  imm=%d\n",
                   ID_EX.pc_of_instr, ID_EX.opcode,
                   ID_EX.r1, (int)ID_EX.val_r1,
                   (int)ID_EX.imm);
        else
            printf("  [LATCH ID/EX] PC=%d  opcode=%d  R%d(%d)  R%d(%d)\n",
                   ID_EX.pc_of_instr, ID_EX.opcode,
                   ID_EX.r1, (int)ID_EX.val_r1,
                   ID_EX.r2, (int)ID_EX.val_r2);
    else
        printf("  [LATCH ID/EX] (bubble)\n");
}
