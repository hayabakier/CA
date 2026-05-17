/* Alu.c — standalone ALU for Package 4
 *
 * NOTE: This file is NOT compiled into the main build (the execute.c
 * pipeline handles ALU operations directly for simplicity).  It is
 * kept here as a reference / alternative entry point.
 *
 * The illegal nested function BREQZ() that was previously placed inside
 * ALU() has been removed; BEQZ branch logic belongs in execute.c where
 * PC and the pipeline state are accessible.
 */

#include <stdio.h>
#include <stdint.h>

/* SREG flags — mirrors of the ones in execute.c (kept here for
 * standalone ALU testing; not extern-linked in the main build) */
static int C_flag = 0;
static int V_flag = 0;
static int N_flag = 0;
static int S_flag = 0;
static int Z_flag = 0;

int ALU(int operandA, int operandB, int operation) {

    int output = 0;
    C_flag = V_flag = N_flag = S_flag = Z_flag = 0;

    switch (operation) {

       /* ADD */
case 0: {
    int temp1 = operandA & 0xFF;
    int temp2 = operandB & 0xFF;
    output = (uint8_t)(temp1 + temp2);
    C_flag = ((temp1 + temp2) & 0x100) == 0x100 ? 1 : 0;
    V_flag = (((operandA >> 7) & 1) == ((operandB >> 7) & 1)) &&
             (((output   >> 7) & 1) != ((operandA >> 7) & 1));
    N_flag = (output >> 7) & 1;
    S_flag = N_flag ^ V_flag;
    Z_flag = (output == 0);
    break;
}
        /* SUB */
        case 1:
            output = (uint8_t)((uint8_t)operandA - (uint8_t)operandB);
            V_flag = (((operandA >> 7) & 1) != ((operandB >> 7) & 1)) &&
                     (((output   >> 7) & 1) == ((operandB >> 7) & 1));
            N_flag = (output >> 7) & 1;
            S_flag = N_flag ^ V_flag;
            Z_flag = (output == 0);
            break;

        /* MUL */
        case 2:
            output = (uint8_t)((uint8_t)operandA * (uint8_t)operandB);
            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0);
            break;

        /* AND */
        case 3:
            output = (uint8_t)operandA & (uint8_t)operandB;
            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0);
            break;

        /* OR */
        case 4:
            output = (uint8_t)operandA | (uint8_t)operandB;
            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0);
            break;

        /* SAL */
        case 5:
            output = (uint8_t)((uint8_t)operandA << operandB);
            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0);
            break;

        /* SAR */
        case 6:
            output = (uint8_t)((int8_t)operandA >> operandB);
            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0);
            break;

        default:
            printf("Invalid operation code: %d\n", operation);
            return 0;
    }

    printf("Operation = %d  A=%d  B=%d  Result=%d  C=%d V=%d N=%d S=%d Z=%d\n",
           operation, operandA, operandB, output,
           C_flag, V_flag, N_flag, S_flag, Z_flag);

    return output;
}
