#include <stdio.h>
#include <stdint.h>

/* ── SREG flags (global, like zeroFlag in your example) ────── */
int C_flag = 0;   /* Carry    – updated by ADD only            */
int V_flag = 0;   /* Overflow – updated by ADD, SUB            */
int N_flag = 0;   /* Negative – updated by ADD,SUB,MUL,AND,OR,SAL,SAR */
int S_flag = 0;   /* Sign     – updated by ADD, SUB  (S = N^V) */
int Z_flag = 0;   /* Zero     – updated by ADD,SUB,MUL,AND,OR,SAL,SAR */


int ALU(int operandA, int operandB, int operation) {

    int output = 0;

    C_flag = 0;
    V_flag = 0;
    N_flag = 0;
    S_flag = 0;
    Z_flag = 0;

    switch (operation) {

        /* ── ADD ───────────────────────────────────────────── */
        case 0:
            output = (uint8_t)((uint8_t)operandA + (uint8_t)operandB);

            C_flag = (((unsigned int)operandA + (unsigned int)operandB) >> 8) & 1;

            V_flag = (((operandA >> 7) & 1) == ((operandB >> 7) & 1)) &&
                     (((output   >> 7) & 1) != ((operandA >> 7) & 1));

            N_flag = (output >> 7) & 1;
            S_flag = N_flag ^ V_flag;
            Z_flag = (output == 0) ? 1 : 0;
            break;

        /* ── SUB ───────────────────────────────────────────── */
        case 1:
            output = (uint8_t)((uint8_t)operandA - (uint8_t)operandB);

            V_flag = (((operandA >> 7) & 1) != ((operandB >> 7) & 1)) &&  (((output   >> 7) & 1) == ((operandB >> 7) & 1));

            N_flag = (output >> 7) & 1;
            S_flag = N_flag ^ V_flag;
            Z_flag = (output == 0) ? 1 : 0;
            break;

        /* ── MUL ───────────────────────────────────────────── */
        case 2:
            output = (uint8_t)((uint8_t)operandA * (uint8_t)operandB);

            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0) ? 1 : 0;
            break;

        /* ── AND ───────────────────────────────────────────── */
        case 3:
            output = (uint8_t)operandA & (uint8_t)operandB;

            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0) ? 1 : 0;
            break;

        /* ── OR ────────────────────────────────────────────── */
        case 4:
            output = (uint8_t)operandA | (uint8_t)operandB;

            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0) ? 1 : 0;
            break;

        /* ── SAL (Shift Arithmetic Left) ───────────────────── */
        case 5:
            output = (uint8_t)((uint8_t)operandA << operandB);

            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0) ? 1 : 0;
            break;

        /* ── SAR (Shift Arithmetic Right) ──────────────────── */
        case 6:
            output = (uint8_t)((int8_t)operandA >> operandB);

            N_flag = (output >> 7) & 1;
            Z_flag = (output == 0) ? 1 : 0;
            break;


        default:
            printf("Invalid operation code.\n");
    }
    int BREQZ(int operandA, int imm) {
 
    printf("BREQZ:\n");
    printf("R1          = %d\n", operandA);
    printf("IMM         = %d\n", imm);
    printf("Current PC  = %d\n", PC);
 
    if (operandA == 0) {
        PC = PC + 1 + imm;
        printf("Branch TAKEN  ->  new PC = %d\n", PC);
        return 1;
    } else {
        PC = PC + 1;
        printf("Branch NOT taken  ->  PC = %d\n", PC);
        return 0;
    }
}
 

    printf("Operation   = %d\n", operation);
    printf("Operand A   = %d\n", operandA);
    printf("Operand B   = %d\n", operandB);
    printf("Result      = %d\n", output);
    printf("C=%d  V=%d  N=%d  S=%d  Z=%d\n",
           C_flag, V_flag, N_flag, S_flag, Z_flag);

    return output;
}
