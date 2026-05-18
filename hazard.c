
#include "defs.h"

int stall_pipeline = 0;


int detectHazard(void)
{
    stall_pipeline = 0;

    if (!ID_EX.valid || !IF_ID.valid)
        return 0;

    int ex_writes = (ID_EX.opcode != SB  &&
                     ID_EX.opcode != BEQZ &&
                     ID_EX.opcode != JR);
    if (!ex_writes) return 0;

    int ex_dest = ID_EX.r1;   

    int reads_r1 = 0, reads_r2 = 0;
    switch (IF_ID.opcode) {
        case ADD: case SUB: case MUL: case AND: case OR: case JR:
            reads_r1 = 1; reads_r2 = 1; break;
        case LDI: case SAL: case SAR: case LB: case BEQZ:
            reads_r1 = 1; reads_r2 = 0; break;
        case SB:
            reads_r1 = 1; reads_r2 = 0; break;   
        default:
            break;
    }

    int hazard_r1 = reads_r1 && (IF_ID.r1 == ex_dest);
    int hazard_r2 = reads_r2 && (IF_ID.r2 == ex_dest);

    if (!hazard_r1 && !hazard_r2) return 0;

    
    if (ID_EX.opcode == LB) {
        printf("[HAZ | cycle %d] LOAD-USE: LB dest=R%d needed by next instr"
               " — inserting bubble (stall)\n",
               current_cycle, ex_dest);
        stall_pipeline = 1;
        return 1;
    }

    printf("[HAZ | cycle %d] RAW hazard: EX writing R%d, ID reads R%d"
           " — forwarding will resolve\n",
           current_cycle, ex_dest, ex_dest);
    return 0;
}
