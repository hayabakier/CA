#include <stdio.h>
#include "hazard.h"
#include "pipeline.h"
#include "parser.h"

int stall_pipeline = 0;

int detectHazard(void)
{
    if (!ID_EX.valid || !IF_ID.valid)
        return 0;

    int exwritesreg = (ID_EX.opcode != SB && ID_EX.opcode != BEQZ && ID_EX.opcode != JR);

    if (!exwritesreg)
        return 0;

    int exdest = ID_EX.r1;

    int idr1 = 0;
    int idr2 = 0;

    if (IF_ID.opcode == ADD || IF_ID.opcode == SUB || IF_ID.opcode == MUL || IF_ID.opcode == AND || IF_ID.opcode == OR || IF_ID.opcode == JR)
    {
        idr1 = 1;
        idr2 = 1;
    }
    else if (IF_ID.opcode == LDI || IF_ID.opcode == SAL || IF_ID.opcode == SAR || IF_ID.opcode == LB || IF_ID.opcode == SB || IF_ID.opcode == BEQZ)
    {
        idr1 = 1;
        idr2 = 0;
    }

    int hazardonr1 = (idr1 && IF_ID.r1 == exdest);
    int hazardonr2 = (idr2 && IF_ID.r2 == exdest);

    if (hazardonr1 || hazardonr2)
    {
        printf("Hazard detected: EX is writing to R%d but ID needs to read R%d, forwarding will fix this\n", exdest, exdest);
        return 0;
    }

    return 0;
}