/* =======================================================================
 * hazard.c — RAW (Read-After-Write) data hazard detection
 *
 * Package 4 has only 3 stages (IF → ID → EX).
 * An instruction in EX is writing a result; the instruction behind it
 * (now in ID) may need that result as a source operand.
 *
 * For Package 4 forwarding covers this 1-cycle gap — so we detect the
 * hazard, print a notice, and let applyForwarding() fix the values.
 * A stall is only needed for a load-use hazard (LB followed immediately
 * by an instruction that reads the loaded register), because the load
 * data isn't available until EX completes.
 * ======================================================================= */
#include "defs.h"

int stall_pipeline = 0;

/*
 * Returns 1 if a STALL (bubble) must be inserted (load-use only).
 * Returns 0 otherwise (forwarding handles it, no stall needed).
 *
 * Must be called after IF_ID is filled (during ID stage) and while
 * ID_EX still holds the previous instruction.
 */
int detectHazard(void)
{
    stall_pipeline = 0;

    /* Need both latches to be valid */
    if (!ID_EX.valid || !IF_ID.valid)
        return 0;

    /* Does the EX-stage instruction write a register? */
    int ex_writes = (ID_EX.opcode != SB  &&
                     ID_EX.opcode != BEQZ &&
                     ID_EX.opcode != JR);
    if (!ex_writes) return 0;

    int ex_dest = ID_EX.r1;   /* destination register in EX */

    /* What registers does the incoming ID instruction read? */
    int reads_r1 = 0, reads_r2 = 0;
    switch (IF_ID.opcode) {
        case ADD: case SUB: case MUL: case AND: case OR: case JR:
            reads_r1 = 1; reads_r2 = 1; break;
        case LDI: case SAL: case SAR: case LB: case BEQZ:
            reads_r1 = 1; reads_r2 = 0; break;
        case SB:
            reads_r1 = 1; reads_r2 = 0; break;   /* SB R1 imm → reads r1 */
        default:
            break;
    }

    int hazard_r1 = reads_r1 && (IF_ID.r1 == ex_dest);
    int hazard_r2 = reads_r2 && (IF_ID.r2 == ex_dest);

    if (!hazard_r1 && !hazard_r2) return 0;

    /* Load-use: LB result only ready after EX → must stall 1 cycle */
    if (ID_EX.opcode == LB) {
        printf("[HAZ | cycle %d] LOAD-USE: LB dest=R%d needed by next instr"
               " — inserting bubble (stall)\n",
               current_cycle, ex_dest);
        stall_pipeline = 1;
        return 1;
    }

    /* All other RAW hazards: forwarding from EX to ID resolves it */
    printf("[HAZ | cycle %d] RAW hazard: EX writing R%d, ID reads R%d"
           " — forwarding will resolve\n",
           current_cycle, ex_dest, ex_dest);
    return 0;
}
