/* parser.c — encodes assembly instructions into 16-bit words */
#include "defs.h"

short int encodeRType(int opcode, int r1, int r2) {
    return (short int)((opcode << 12) | (r1 << 6) | r2);
}

short int encodeIType(int opcode, int r1, int imm) {
    return (short int)((opcode << 12) | (r1 << 6) | (imm & 0x3F));
}

void parseFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { printf("Error opening file: %s\n", filename); return; }

    char line[100];
    int  idx = 0, r1, r2, imm, valid;
    short int enc;

    while (fgets(line, sizeof(line), file)) {
        valid = 1;
        if      (sscanf(line, "ADD R%d R%d",  &r1, &r2)  == 2) enc = encodeRType(ADD,  r1, r2);
        else if (sscanf(line, "SUB R%d R%d",  &r1, &r2)  == 2) enc = encodeRType(SUB,  r1, r2);
        else if (sscanf(line, "MUL R%d R%d",  &r1, &r2)  == 2) enc = encodeRType(MUL,  r1, r2);
        else if (sscanf(line, "AND R%d R%d",  &r1, &r2)  == 2) enc = encodeRType(AND,  r1, r2);
        else if (sscanf(line, "OR R%d R%d",   &r1, &r2)  == 2) enc = encodeRType(OR,   r1, r2);
        else if (sscanf(line, "JR R%d R%d",   &r1, &r2)  == 2) enc = encodeRType(JR,   r1, r2);
        else if (sscanf(line, "LDI R%d %d",   &r1, &imm) == 2) enc = encodeIType(LDI,  r1, imm);
        else if (sscanf(line, "BEQZ R%d %d",  &r1, &imm) == 2) enc = encodeIType(BEQZ, r1, imm);
        else if (sscanf(line, "SAL R%d %d",   &r1, &imm) == 2) enc = encodeIType(SAL,  r1, imm);
        else if (sscanf(line, "SAR R%d %d",   &r1, &imm) == 2) enc = encodeIType(SAR,  r1, imm);
        else if (sscanf(line, "LB R%d %d",    &r1, &imm) == 2) enc = encodeIType(LB,   r1, imm);
        else if (sscanf(line, "SB R%d %d",    &r1, &imm) == 2) enc = encodeIType(SB,   r1, imm);
        else valid = 0;

        if (valid) {
            instructionMemory[idx++] = enc;
            NumberofInstructions++;
        }
    }
    fclose(file);
    printf("Parsed %d instructions.\n", NumberofInstructions);
}
